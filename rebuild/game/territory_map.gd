extends Control
signal selected(kind: String, id: int)
var world: Dictionary = {}
var selected_site := 10
var selected_person := 1
var font: Font
const GOLD = Color("dfbf7a")
const TEAL = Color("67c6b2")
const INK = Color("101f25")
const NAMES = {10:"CUNG ĐIỆN",11:"RUỘNG",12:"XƯỞNG",13:"CHỢ",14:"GIẾNG CÔNG",15:"NHÀ Ở"}
func _ready() -> void:
	font = ThemeDB.fallback_font
	mouse_default_cursor_shape = Control.CURSOR_POINTING_HAND
func point(l: Dictionary) -> Vector2:
	return Vector2(float(l.x) / 680.0 * size.x, 40 + float(l.y) / 580.0 * (size.y-70))
func loc_by_id(id: int) -> Dictionary:
	for l in world.get("locations", []):
		if int(l.id) == id:
			return l
	return {}
func npc_point(p: Dictionary) -> Vector2:
	var l := loc_by_id(int(p.location))
	if l.is_empty():
		return Vector2.ZERO
	var pos := point(l) + Vector2((int(p.id)%3-1)*27, 36 + int(p.id)/3*19)
	if p.get("activity", "") == "Travel":
		var to := loc_by_id(int(p.target))
		if not to.is_empty():
			pos = pos.lerp(point(to), 0.45)
	return pos
func _draw() -> void:
	draw_style_box(panel(), Rect2(Vector2.ZERO,size))
	if world.is_empty():
		return
	# A single draw surface; no scene/model object per person.
	var river := PackedVector2Array([Vector2(size.x*.04,0),Vector2(size.x*.08,size.y*.25),Vector2(size.x*.03,size.y*.52),Vector2(size.x*.12,size.y*.8),Vector2(size.x*.08,size.y)])
	draw_polyline(river, Color("214756"), 24, true)
	for i in range(30):
		var x := fmod(float(i*137+75),size.x-100)+50
		var y := fmod(float(i*73+80),size.y-120)+65
		draw_circle(Vector2(x,y),2,Color("263d3b"))
	for edge in [[10,11],[10,12],[11,13],[12,13],[13,14],[13,15]]:
		draw_line(point(loc_by_id(edge[0])),point(loc_by_id(edge[1])),Color("45504a"),3,true)
	for l in world.locations:
		var pos := point(l)
		var chosen := int(l.id)==selected_site
		draw_circle(pos,35,Color("293e3a") if chosen else INK)
		draw_arc(pos,35,0,TAU,40,GOLD if chosen else Color("547265"),2,true)
		if int(l.id)==11:
			for row in range(4):
				draw_line(pos+Vector2(-19,-12+row*8),pos+Vector2(19,-12+row*8),GOLD,2)
		elif int(l.id)==14:
			draw_arc(pos,16,0,TAU,24,TEAL,3,true)
			draw_line(pos+Vector2(-20,-16),pos+Vector2(20,-16),GOLD,3)
		else:
			draw_rect(Rect2(pos+Vector2(-18,-10),Vector2(36,27)),Color("5e786d"))
			draw_colored_polygon(PackedVector2Array([pos+Vector2(-24,-9),pos+Vector2(0,-27),pos+Vector2(24,-9)]),GOLD)
			draw_rect(Rect2(pos+Vector2(-4,2),Vector2(8,15)),INK)
		var name: String = NAMES[int(l.id)]
		draw_string(font,pos+Vector2(-font.get_string_size(name,HORIZONTAL_ALIGNMENT_LEFT,-1,15).x/2,-43),name,HORIZONTAL_ALIGNMENT_LEFT,-1,15,GOLD)
		if l.get("job",false):
			draw_circle(pos+Vector2(32,-25),5,TEAL)
	for p in world.people:
		var pos := npc_point(p)
		var selected_npc := int(p.id)==selected_person
		draw_circle(pos,10,TEAL if int(p.id)>3 else GOLD)
		if selected_npc:
			draw_arc(pos,15,0,TAU,24,Color.WHITE,1.5,true)
		draw_string(font,pos+Vector2(-4,4),str(int(p.id)),HORIZONTAL_ALIGNMENT_LEFT,-1,12,INK)
		if p.has("activity") and p.activity!="Idle":
			var w := 44.0
			draw_rect(Rect2(pos+Vector2(-22,15),Vector2(w,4)),INK)
			draw_rect(Rect2(pos+Vector2(-22,15),Vector2(w*min(1.0,float(p.progress)/max(1.0,float(p.required))),4)),TEAL)
	draw_string(font,Vector2(22,30),"THANH ĐIỀN   /   01 LÃNH THỔ",HORIZONTAL_ALIGNMENT_LEFT,-1,18,GOLD)
	draw_string(font,Vector2(22,size.y-18),"● Việc đã đăng     1–5 Cá nhân     Chọn địa điểm hoặc người để xem",HORIZONTAL_ALIGNMENT_LEFT,-1,13,Color("9cafab"))
func panel() -> StyleBoxFlat:
	var s := StyleBoxFlat.new()
	s.bg_color=Color("142b2d")
	s.corner_radius_top_left=12
	s.corner_radius_top_right=12
	s.corner_radius_bottom_left=12
	s.corner_radius_bottom_right=12
	return s
func _gui_input(event: InputEvent) -> void:
	if event is InputEventMouseButton and event.pressed and event.button_index==MOUSE_BUTTON_LEFT:
		for p in world.get("people",[]):
			if npc_point(p).distance_to(event.position)<19:
				selected_person=int(p.id)
				selected.emit("person",selected_person)
				queue_redraw()
				return
		for l in world.get("locations",[]):
			if point(l).distance_to(event.position)<44:
				selected_site=int(l.id)
				selected.emit("site",selected_site)
				queue_redraw()
				return
