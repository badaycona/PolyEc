extends Control
signal selected(kind: String, id: int)
var world: Dictionary={}
var selected_site:=10
var selected_person:=1
var selected_route:=0
var font: Font
const GOLD=Color("dfbf7a")
const TEAL=Color("67c6b2")
const INK=Color("101f25")
func _ready() -> void:
	font=ThemeDB.fallback_font
	mouse_default_cursor_shape=Control.CURSOR_POINTING_HAND
func point(l: Dictionary) -> Vector2:
	var extent:=1400.0 if int(world.get("fixture_regions",1))==2 else 680.0
	return Vector2(float(l.x)/extent*size.x,48+float(l.y)/580.0*(size.y-90))
func loc_by_id(id: int) -> Dictionary:
	for l in world.get("locations",[]):
		if int(l.id)==id:
			return l
	return {}
func route_by_id(id: int) -> Dictionary:
	for r in world.get("routes",[]):
		if int(r.id)==id:
			return r
	return {}
func npc_point(p: Dictionary) -> Vector2:
	var l:=loc_by_id(int(p.location))
	if l.is_empty():
		return Vector2.ZERO
	var journey: Dictionary=p.get("journey",{})
	var route_id: int=int(journey.get("route",p.get("position_route",0)))
	if route_id!=0:
		var r:=route_by_id(route_id)
		if not r.is_empty():
			var fraction: float=float(journey.get("coordinate",p.get("road_coordinate",0)))/float(r.length)
			return point(loc_by_id(int(r.from))).lerp(point(loc_by_id(int(r.to))),fraction)+Vector2(0,-17-int(p.id)%2*13)
	var pos:=point(l)+Vector2((int(p.id)%3-1)*20,30+int(p.id)/3*13)
	if p.get("activity","")=="Travel":
		var to:=loc_by_id(int(p.target))
		if not to.is_empty():
			pos=pos.lerp(point(to),0.45)
	return pos
func caption(l: Dictionary) -> String:
	if int(l.id)==11:
		return "RUỘNG"
	if int(l.id)==21:
		return "BÃI GỖ"
	return {1:"CUNG",2:"CHỢ",3:"GIẾNG",4:"TRỌ" if int(l.region)==2 else "NHÀ",5:"XƯỞNG",6:"TRẠM NGHỈ"}.get(int(l.kind),str(l.name))
func _draw() -> void:
	draw_style_box(panel(),Rect2(Vector2.ZERO,size))
	if world.is_empty():
		return
	var two: bool=int(world.get("fixture_regions",1))==2
	if two:
		draw_rect(Rect2(Vector2(size.x*.52,0),Vector2(size.x*.48,size.y)),Color("1b3032"))
		draw_line(Vector2(size.x*.49,42),Vector2(size.x*.49,size.y-35),Color("547265"),1,true)
	for i in range(30):
		draw_circle(Vector2(fmod(float(i*137+75),size.x-70)+35,fmod(float(i*73+80),size.y-100)+55),2,Color("2c4440"))
	for base in [10,20] if two else [10]:
		for edge in [[0,1],[0,2],[1,3],[2,3],[3,4],[3,5]]:
			draw_line(point(loc_by_id(base+edge[0])),point(loc_by_id(base+edge[1])),Color("45504a"),2,true)
	for r in world.get("routes",[]):
		var a:=point(loc_by_id(int(r.from)))
		var b:=point(loc_by_id(int(r.to)))
		var color:=TEAL if r.get("open",true) else Color("e18a65")
		draw_line(a,b,color,5 if selected_route==int(r.id) else 3,true)
		var middle: Vector2=(a+b)/2+Vector2(-48,44)
		draw_string(font,middle,"120 phút · phí2",HORIZONTAL_ALIGNMENT_LEFT,-1,12,color)
		if not r.get("open",true):
			draw_string(font,middle+Vector2(12,17),"ĐƯỜNG ĐÓNG",HORIZONTAL_ALIGNMENT_LEFT,-1,12,color)
	for l in world.locations:
		if int(l.kind)==7:
			continue
		var pos:=point(l)
		var chosen: bool=int(l.id)==selected_site
		var radius:=23.0 if two else 32.0
		draw_circle(pos,radius,Color("293e3a") if chosen else INK)
		draw_arc(pos,radius,0,TAU,32,GOLD if chosen else Color("547265"),2,true)
		if int(l.id)==11 or int(l.id)==21:
			for row in range(3):
				draw_line(pos+Vector2(-14,-7+row*7),pos+Vector2(14,-7+row*7),GOLD,2)
		elif int(l.kind)==3:
			draw_arc(pos,12,0,TAU,20,TEAL,3,true)
		else:
			draw_rect(Rect2(pos+Vector2(-13,-6),Vector2(26,20)),Color("5e786d"))
			draw_colored_polygon(PackedVector2Array([pos+Vector2(-17,-5),pos+Vector2(0,-19),pos+Vector2(17,-5)]),GOLD)
		var name:=caption(l)
		draw_string(font,pos+Vector2(-font.get_string_size(name,HORIZONTAL_ALIGNMENT_LEFT,-1,12).x/2,-31),name,HORIZONTAL_ALIGNMENT_LEFT,-1,12,GOLD)
		if l.get("job",false):
			draw_circle(pos+Vector2(22,-19),4,TEAL)
	for p in world.people:
		var pos:=npc_point(p)
		draw_circle(pos,9,TEAL if int(p.id)>=4 else GOLD)
		if int(p.id)==selected_person:
			draw_arc(pos,13,0,TAU,24,Color.WHITE,1.5,true)
		draw_string(font,pos+Vector2(-4,4),str(int(p.id)),HORIZONTAL_ALIGNMENT_LEFT,-1,11,INK)
		if p.has("activity") and p.activity!="Idle":
			draw_rect(Rect2(pos+Vector2(-18,13),Vector2(36,3)),INK)
			draw_rect(Rect2(pos+Vector2(-18,13),Vector2(36*min(1.0,float(p.progress)/max(1.0,float(p.required))),3)),TEAL)
	draw_string(font,Vector2(18,28),"THANH ĐIỀN",HORIZONTAL_ALIGNMENT_LEFT,-1,17,GOLD)
	if two:
		draw_string(font,Vector2(size.x*.57,28),"TÙNG LÂM",HORIZONTAL_ALIGNMENT_LEFT,-1,17,GOLD)
	draw_string(font,Vector2(18,size.y-15),"● Job đang mở   · Chọn người / địa điểm / tuyến   · Tải, phí và tiến độ lấy từ core",HORIZONTAL_ALIGNMENT_LEFT,-1,12,Color("9cafab"))
func panel() -> StyleBoxFlat:
	var s:=StyleBoxFlat.new()
	s.bg_color=Color("142b2d")
	s.set_corner_radius_all(12)
	return s
func _gui_input(event: InputEvent) -> void:
	if not (event is InputEventMouseButton and event.pressed and event.button_index==MOUSE_BUTTON_LEFT):
		return
	for p in world.get("people",[]):
		if npc_point(p).distance_to(event.position)<16:
			selected_person=int(p.id)
			selected.emit("person",selected_person)
			queue_redraw()
			return
	for l in world.get("locations",[]):
		if int(l.kind)!=7 and point(l).distance_to(event.position)<29:
			selected_site=int(l.id)
			selected.emit("site",selected_site)
			queue_redraw()
			return
	for r in world.get("routes",[]):
		var close:=Geometry2D.get_closest_point_to_segment(event.position,point(loc_by_id(int(r.from))),point(loc_by_id(int(r.to))))
		if close.distance_to(event.position)<12:
			selected_route=int(r.id)
			selected.emit("route",selected_route)
			queue_redraw()
			return
