extends Control

signal territory_selected(id: int)

var snapshot: Dictionary = {}
var selected: int = 1
var overlay: int = 0
var zoom: float = 1.0
var pan := Vector2.ZERO
var dragging: bool = false
var route_path: Array = []
var marks: Array = []
var name_style: StyleBoxFlat
var font: Font = preload("res://fonts/BeVietnamPro-Regular.ttf")
var title_font: Font = preload("res://fonts/Newsreader.ttf")
const COLORS = ["9baf79", "887b69", "b0a575", "8d967c", "527e68", "a5b397", "b6b27e"]

func _ready() -> void:
    clip_contents = true
    name_style = _name_style()
    resized.connect(reset_view)
    call_deferred("reset_view")

func reset_view() -> void:
    zoom = minf(size.x / 1010.0, size.y / 780.0)
    pan = (size - Vector2(1000, 760) * zoom) / 2.0
    queue_redraw()

func update_world(value: Dictionary) -> void:
    snapshot = value
    if marks.is_empty():
        # Geography is immutable in M4. Do the polygon queries once, rather
        # than thousands of queries and allocations on each input redraw.
        for r in snapshot.get("regions", []):
            var region_marks: Array = []
            for y in range(90, 710, 30):
                for x in range(80, 930, 33):
                    var p = Vector2(x + (y % 13), y)
                    if Geometry2D.is_point_in_polygon(p, r.polygon):
                        region_marks.append(p)
            marks.append(region_marks)
    queue_redraw()

func region_at(position_value: Vector2) -> int:
    var point = (position_value - pan) / zoom
    for r in snapshot.get("regions", []):
        if Geometry2D.is_point_in_polygon(point, r.polygon):
            return int(r.id)
    return -1

func _gui_input(event: InputEvent) -> void:
    if event is InputEventMouseButton:
        if event.button_index in [MOUSE_BUTTON_RIGHT, MOUSE_BUTTON_MIDDLE]:
            dragging = event.pressed
            accept_event()
        elif event.pressed and event.button_index == MOUSE_BUTTON_LEFT:
            var id = region_at(event.position)
            if id >= 0:
                selected = id
                territory_selected.emit(id)
                queue_redraw()
            accept_event()
        elif event.pressed and event.button_index in [MOUSE_BUTTON_WHEEL_UP, MOUSE_BUTTON_WHEEL_DOWN]:
            var previous = zoom
            zoom = clampf(zoom * (1.15 if event.button_index == MOUSE_BUTTON_WHEEL_UP else 1.0 / 1.15), 0.25, 2.5)
            pan = event.position - (event.position - pan) * zoom / previous
            queue_redraw()
            accept_event()
    elif event is InputEventMouseMotion:
        if dragging:
            pan += event.relative
            queue_redraw()
        var id = region_at(event.position)
        tooltip_text = snapshot.regions[id].name + " · " + snapshot.regions[id].terrain if id >= 0 else "Cuộn để phóng · Kéo chuột phải để di chuyển · Home để đặt lại"

func _draw() -> void:
    draw_rect(Rect2(Vector2.ZERO, size), Color("122c35"))
    draw_set_transform(pan, 0, Vector2.ONE * zoom)
    for y in range(50, 760, 45):
        draw_line(Vector2(0, y), Vector2(1000, y), Color(0.3, 0.6, 0.65, 0.08), 1)
    for r in snapshot.get("regions", []):
        var color = Color(COLORS[r.id])
        if overlay == 1:
            color = Color("ad6750").lerp(Color("658b6a"), clampf(float(r.cover) / 50.0, 0, 1))
        elif overlay == 2:
            color = Color("6a8178")
            for k in snapshot.get("cases", []):
                if k.region == r.id and k.closed < 0:
                    color = Color("b7784d")
        draw_colored_polygon(r.polygon, color)
        var border: PackedVector2Array = r.polygon.duplicate()
        border.append(border[0])
        draw_polyline(border, Color("263e3c"), 2, true)
        # Local deterministic cartography: no downloaded assets or world state.
        for p in marks[r.id]:
            if int(r.id) in [1, 3]:
                var peak = PackedVector2Array([p + Vector2(-11, 8), p + Vector2(0, -12), p + Vector2(12, 8)])
                draw_colored_polygon(peak, Color(0.2, 0.25, 0.23, 0.24))
                draw_line(p + Vector2(0, -12), p + Vector2(12, 8), Color(0.9, 0.88, 0.7, 0.32), 2, true)
            elif int(r.id) == 4:
                draw_circle(p, 5, Color(0.1, 0.27, 0.2, 0.3))
            else:
                draw_line(p, p + Vector2(15, -3), Color(0.2, 0.32, 0.2, 0.18), 1, true)
    var river = PackedVector2Array([Vector2(690, 100), Vector2(620, 220), Vector2(580, 290), Vector2(600, 350), Vector2(700, 435), Vector2(810, 560), Vector2(890, 650)])
    draw_polyline(river, Color("284f57"), 13, true)
    draw_polyline(river, Color("8bbfbc"), 7, true)
    for route in snapshot.get("routes", []):
        var a: Vector2 = snapshot.regions[route.from].center
        var b: Vector2 = snapshot.regions[route.to].center
        var color = Color("e2c995") if route.open else Color("834b42")
        if route.claimed:
            color = Color("b36342")
        draw_line(a, b, Color("3e493d"), 6, true)
        draw_dashed_line(a, b, color, 2.5, 10, true)
        if route.works_due > 0:
            draw_circle((a + b) / 2.0, 8, Color("d3a355"))
    for i in range(route_path.size() - 1):
        draw_line(snapshot.regions[route_path[i]].center, snapshot.regions[route_path[i + 1]].center, Color("f4e5ac"), 4, true)
    for cargo in snapshot.get("cargo", []):
        if cargo.delivered or cargo.path.size() < 2:
            continue
        var position_value: Vector2 = snapshot.regions[cargo.path[0]].center
        for leg in range(cargo.path.size() - 1):
            var a: Vector2 = snapshot.regions[cargo.path[leg]].center
            var b: Vector2 = snapshot.regions[cargo.path[leg + 1]].center
            if snapshot.day < cargo.starts[leg]:
                position_value = a
                break
            if snapshot.day < cargo.arrivals[leg]:
                var progress = clampf(float(snapshot.day - cargo.ends[leg]) / maxf(1, cargo.arrivals[leg] - cargo.ends[leg]), 0, 1)
                position_value = a.lerp(b, progress)
                break
            position_value = b
        draw_circle(position_value, 7, Color("fff1bc"))
    for r in snapshot.get("regions", []):
        var p: Vector2 = r.center
        if r.id == selected:
            var border: PackedVector2Array = r.polygon.duplicate()
            border.append(border[0])
            draw_polyline(border, Color("f8e6ad"), 4, true)
        draw_circle(p, 8, Color("233e38"))
        draw_circle(p, 3, Color("f1dfaa"))
        var width = title_font.get_string_size(r.name, HORIZONTAL_ALIGNMENT_LEFT, -1, 24).x
        draw_style_box(name_style, Rect2(p + Vector2(-width / 2.0 - 8, 12), Vector2(width + 16, 33)))
        draw_string(title_font, p + Vector2(-width / 2.0, 36), r.name, HORIZONTAL_ALIGNMENT_LEFT, -1, 24, Color("f7edcf"))
        if overlay == 1:
            draw_string(font, p + Vector2(-38, 61), "%d ngày kho" % r.cover, HORIZONTAL_ALIGNMENT_LEFT, -1, 13, Color("102d2b"))
    draw_string(title_font, Vector2(860, 745), "Vịnh Vân", HORIZONTAL_ALIGNMENT_LEFT, -1, 24, Color("8fbbb9"))
    draw_set_transform(Vector2.ZERO)

func _name_style() -> StyleBoxFlat:
    var style = StyleBoxFlat.new()
    style.bg_color = Color(0.07, 0.16, 0.17, 0.92)
    style.set_corner_radius_all(5)
    return style
