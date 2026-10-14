extends Control

signal territory_selected(id: int)

const INK = Color("0b191f")
const GOLD = Color("d1b575")
const PAPER = Color("e7e2cd")
const JADE = Color("74b7a5")
var POLYGONS = [
    PackedVector2Array([Vector2(120,155),Vector2(300,95),Vector2(480,110),Vector2(535,190),Vector2(500,280),Vector2(565,375),Vector2(505,485),Vector2(335,570),Vector2(185,535),Vector2(105,420),Vector2(75,285)]),
    PackedVector2Array([Vector2(480,110),Vector2(680,70),Vector2(820,145),Vector2(910,260),Vector2(885,395),Vector2(800,500),Vector2(650,555),Vector2(505,485),Vector2(565,375),Vector2(500,280),Vector2(535,190)])
]
const IDS = [7,8]
var selected_id: int = 7
var hovered_id: int = 0
var zoom: float = 1.0
var pan = Vector2.ZERO
var show_routes: bool = true
var dragging: bool = false
var regular: Font = preload("res://fonts/BeVietnamPro-Regular.ttf")
var serif: Font = preload("res://fonts/Newsreader.ttf")
var texture_marks: Array = []
var trees: Array[Vector2] = []
var mountains: Array[Vector2] = []
var river: PackedVector2Array
var road: PackedVector2Array
var _was_sized = false

func _ready() -> void:
    clip_contents = true
    mouse_default_cursor_shape = Control.CURSOR_POINTING_HAND
    var rng = RandomNumberGenerator.new()
    rng.seed = 4027
    var noise = FastNoiseLite.new()
    noise.seed = 87
    noise.frequency = 0.012
    for i in range(4500):
        var p = Vector2(rng.randf_range(65,920),rng.randf_range(65,580))
        for region in range(2):
            if Geometry2D.is_point_in_polygon(p,POLYGONS[region]):
                var n = noise.get_noise_2d(p.x,p.y)
                texture_marks.append([p,region,n,rng.randf_range(0.7,2.1)])
                break
    for i in range(180):
        var p = Vector2(rng.randf_range(140,800),rng.randf_range(140,510))
        if Geometry2D.is_point_in_polygon(p,POLYGONS[0]) and p.distance_to(Vector2(320,360))>65:
            trees.append(p)
    for i in range(30):
        var p = Vector2(rng.randf_range(620,825),rng.randf_range(180,435))
        if Geometry2D.is_point_in_polygon(p,POLYGONS[1]):
            mountains.append(p)
    mountains.sort_custom(func(a,b): return a.y < b.y)
    river = _smooth([Vector2(687,104),Vector2(610,177),Vector2(584,252),Vector2(521,316),Vector2(481,386),Vector2(391,462),Vector2(271,546),Vector2(182,586)])
    road = _smooth([Vector2(289,349),Vector2(382,335),Vector2(472,297),Vector2(560,281),Vector2(649,309),Vector2(731,328)])
    resized.connect(_on_resize)
    mouse_exited.connect(func(): hovered_id=0; queue_redraw())
    call_deferred("reset_view")

func _smooth(points: Array) -> PackedVector2Array:
    var curve = Curve2D.new()
    for i in range(points.size()):
        var before: Vector2 = points[maxi(0,i-1)]
        var after: Vector2 = points[mini(points.size()-1,i+1)]
        var tangent = (after-before)*0.15
        curve.add_point(points[i],-tangent,tangent)
    return curve.get_baked_points()

func _on_resize() -> void:
    reset_view()

func reset_view() -> void:
    zoom = minf(size.x/1040.0,size.y/680.0)
    pan = (size-Vector2(1000,650)*zoom)*0.5
    _was_sized = true
    queue_redraw()

func set_selected(id: int) -> void:
    if selected_id != id:
        selected_id=id
        queue_redraw()

func region_at(local_position: Vector2) -> int:
    var point = (local_position-pan)/zoom
    for i in range(POLYGONS.size()):
        if Geometry2D.is_point_in_polygon(point,POLYGONS[i]): return IDS[i]
    return 0

func _gui_input(event: InputEvent) -> void:
    if event is InputEventMouseButton:
        if event.button_index==MOUSE_BUTTON_MIDDLE or event.button_index==MOUSE_BUTTON_RIGHT:
            dragging=event.pressed
            mouse_default_cursor_shape=Control.CURSOR_DRAG if dragging else Control.CURSOR_POINTING_HAND
            accept_event()
        elif event.pressed and event.button_index==MOUSE_BUTTON_LEFT:
            var id = region_at(event.position)
            if id!=0: territory_selected.emit(id)
            accept_event()
        elif event.pressed and (event.button_index==MOUSE_BUTTON_WHEEL_UP or event.button_index==MOUSE_BUTTON_WHEEL_DOWN):
            var multiplier = 1.13 if event.button_index==MOUSE_BUTTON_WHEEL_UP else 1.0/1.13
            var previous = zoom
            zoom = clampf(zoom*multiplier,0.5,2.5)
            pan = event.position-(event.position-pan)*zoom/previous
            queue_redraw()
            accept_event()
    elif event is InputEventMouseMotion:
        if dragging:
            pan+=event.relative
        hovered_id=region_at(event.position)
        tooltip_text = "Thanh Điền · Chọn để xem hồ sơ" if hovered_id==7 else ("Hắc Sơn · Chọn để xem hồ sơ" if hovered_id==8 else "Kéo chuột giữa/phải để di chuyển bản đồ")
        queue_redraw()

func _draw() -> void:
    draw_rect(Rect2(Vector2.ZERO,size),INK)
    # Fine cartographic grid stays behind the land.
    for x in range(0,int(size.x),48): draw_line(Vector2(x,0),Vector2(x,size.y),Color(0.25,0.40,0.43,0.08),1)
    for y in range(0,int(size.y),48): draw_line(Vector2(0,y),Vector2(size.x,y),Color(0.25,0.40,0.43,0.08),1)
    draw_set_transform(pan,0,Vector2(zoom,zoom))
    for i in range(2):
        var shadow = PackedVector2Array()
        for point in POLYGONS[i]: shadow.append(point+Vector2(4,10))
        draw_colored_polygon(shadow,Color(0.0,0.0,0.0,0.35))
    for i in range(2):
        var fill = Color("24433d") if i==0 else Color("3d4440")
        if IDS[i]==hovered_id: fill=fill.lightened(0.08)
        draw_colored_polygon(POLYGONS[i],fill)
    for mark in texture_marks:
        var col = Color(0.46,0.65,0.43,0.12) if mark[1]==0 else Color(0.67,0.64,0.47,0.16)
        if mark[2]<-0.15: col=Color(0.05,0.12,0.13,0.19)
        draw_circle(mark[0],mark[3],col)
    # Contour-like terraced rings, clipped to actual region polygons.
    for i in range(9):
        var contour = PackedVector2Array()
        for step in range(85):
            var a = float(step)/84.0*TAU
            var r = 36+i*16+sin(a*4+i)*5
            var p = Vector2(731,289)+Vector2(cos(a)*r,sin(a)*r*0.68)
            if Geometry2D.is_point_in_polygon(p,POLYGONS[1]): contour.append(p)
        if contour.size()>2: draw_polyline(contour,Color(0.75,0.74,0.58,0.12),1,true)
    # Field parcels in the cultivated western lowland.
    for row in range(5):
        for col in range(7):
            var p = Vector2(207+col*26,373+row*22)
            if Geometry2D.is_point_in_polygon(p,POLYGONS[0]):
                draw_polyline(PackedVector2Array([p,p+Vector2(19,-5),p+Vector2(24,10)]),Color(0.69,0.75,0.42,0.18),1,true)
    for p in trees:
        draw_line(p,p+Vector2(0,5),Color("142b27"),1)
        draw_polyline(PackedVector2Array([p+Vector2(-3,1),p+Vector2(0,-5),p+Vector2(3,1)]),Color(0.40,0.59,0.40,0.55),1.5,true)
    for p in mountains:
        draw_colored_polygon(PackedVector2Array([p+Vector2(-16,12),p+Vector2(0,-21),p+Vector2(20,12)]),Color("303936"))
        draw_polyline(PackedVector2Array([p+Vector2(-16,12),p+Vector2(0,-21),p+Vector2(20,12)]),Color("a5a68a"),1.2,true)
        draw_polyline(PackedVector2Array([p+Vector2(0,-21),p+Vector2(-2,-6),p+Vector2(8,12)]),Color("657369"),1.1,true)
    draw_polyline(river,Color("18343c"),15,true)
    draw_polyline(river,Color("548f96"),7,true)
    draw_polyline(river,Color(0.52,0.76,0.74,0.6),2,true)
    for i in range(2):
        var outline = PackedVector2Array(POLYGONS[i])
        outline.append(POLYGONS[i][0])
        draw_polyline(outline,Color("0c2427"),5,true)
        draw_polyline(outline,GOLD if selected_id==IDS[i] else Color("70827a"),2.3 if selected_id==IDS[i] else 1.0,true)
    if show_routes:
        draw_polyline(road,Color(0.04,0.09,0.09,0.7),6,true)
        for i in range(0,road.size()-1,3):
            draw_line(road[i],road[mini(i+1,road.size()-1)],Color("cfb87d"),2,true)
        _label(Vector2(452,266),"ĐƯỜNG THIÊN LÝ",11,GOLD,regular)
        draw_circle(Vector2(560,281),6,INK)
        draw_circle(Vector2(560,281),3,GOLD)
    _settlement(Vector2(289,349),"Bến Lúa",selected_id==7)
    _settlement(Vector2(731,328),"Ải Đá",selected_id==8)
    _label(Vector2(304,252),"THANH ĐIỀN",29,PAPER,serif)
    _label(Vector2(304,274),"ĐỒNG BẰNG PHÍA TÂY",10,JADE,regular)
    _label(Vector2(714,435),"HẮC SƠN",30,PAPER,serif)
    _label(Vector2(714,457),"MIỀN NÚI PHÍA ĐÔNG",10,Color("c3b78f"),regular)
    _label(Vector2(363,512),"Sông Thanh",17,Color("86b5b6"),serif)
    _label(Vector2(196,130),"RỪNG TÂY",10,Color("809b87"),regular)
    _label(Vector2(775,157),"DÃY HẮC SƠN",10,Color("a9ad98"),regular)
    # A simple engraved compass.
    var center=Vector2(906,102)
    draw_circle(center,27,Color(0.04,0.10,0.12,0.5))
    draw_arc(center,26,0,TAU,64,Color(0.64,0.65,0.49,0.6),1,true)
    draw_colored_polygon(PackedVector2Array([center+Vector2(0,-22),center+Vector2(6,3),center,center+Vector2(-6,3)]),GOLD)
    draw_colored_polygon(PackedVector2Array([center+Vector2(0,22),center+Vector2(5,-2),center,center+Vector2(-5,-2)]),Color("60777a"))
    _label(center+Vector2(0,-35),"B",12,GOLD,regular)
    draw_set_transform(Vector2.ZERO,0,Vector2.ONE)
    draw_string(regular,Vector2(24,30),"BẢN ĐỒ ĐỊA THẾ",HORIZONTAL_ALIGNMENT_LEFT,-1,11,Color("8da29e"))
    draw_string(serif,Vector2(24,58),"Hai miền, một đường thiên lý",HORIZONTAL_ALIGNMENT_LEFT,-1,23,PAPER)
    draw_line(Vector2(24,size.y-34),Vector2(104,size.y-34),Color("78938e"),1)
    draw_line(Vector2(24,size.y-39),Vector2(24,size.y-29),Color("78938e"),1)
    draw_line(Vector2(104,size.y-39),Vector2(104,size.y-29),Color("78938e"),1)
    draw_string(regular,Vector2(24,size.y-14),"ĐỊA ĐỒ NGUYÊN MẪU",HORIZONTAL_ALIGNMENT_LEFT,-1,9,Color("78938e"))
    draw_rect(Rect2(190,size.y-40,8,8),GOLD)
    draw_string(regular,Vector2(207,size.y-32),"Miền đang chọn",HORIZONTAL_ALIGNMENT_LEFT,-1,10,Color("91a7a4"))
    draw_line(Vector2(335,size.y-36),Vector2(351,size.y-36),Color("548f96"),3,true)
    draw_string(regular,Vector2(360,size.y-32),"Sông",HORIZONTAL_ALIGNMENT_LEFT,-1,10,Color("91a7a4"))
    if show_routes:
        draw_line(Vector2(413,size.y-36),Vector2(429,size.y-36),GOLD,2,true)
        draw_string(regular,Vector2(438,size.y-32),"Đường thiên lý",HORIZONTAL_ALIGNMENT_LEFT,-1,10,Color("91a7a4"))
    draw_string(regular,Vector2(size.x-210,size.y-18),"CUỘN: THU PHÓNG  ·  KÉO: DI CHUYỂN",HORIZONTAL_ALIGNMENT_LEFT,-1,9,Color("78938e"))

func _label(at: Vector2,text: String,font_size: int,color: Color,font: Font) -> void:
    var width = font.get_string_size(text,HORIZONTAL_ALIGNMENT_LEFT,-1,font_size).x
    draw_string(font,at-Vector2(width*0.5,0),text,HORIZONTAL_ALIGNMENT_LEFT,-1,font_size,color)

func _settlement(at: Vector2,text: String,active: bool) -> void:
    draw_circle(at,12,INK)
    draw_arc(at,12,0,TAU,40,GOLD if active else Color("88968b"),1.5,true)
    draw_rect(Rect2(at-Vector2(4,4),Vector2(8,8)),GOLD if active else PAPER)
    _label(at+Vector2(0,31),text,15,PAPER,regular)
