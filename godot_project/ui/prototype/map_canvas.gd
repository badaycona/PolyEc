extends Control

signal territory_selected(id: int)

const INK = Color("0b191f")
const GOLD = Color("d1b575")
const PAPER = Color("e7e2cd")
const MUTED = Color("91a7a4")
var regular: Font = preload("res://fonts/BeVietnamPro-Regular.ttf")
var serif: Font = preload("res://fonts/Newsreader.ttf")
var regions: Array[Dictionary] = []
var links: Array[Dictionary] = []
var settlements: Array[Dictionary] = []
var selected_id: int = 7
var hovered_id: int = -1
var zoom: float = 1.0
var pan := Vector2.ZERO
var show_routes := true
var dragging := false

func _ready() -> void:
    clip_contents = true
    mouse_default_cursor_shape = Control.CURSOR_POINTING_HAND
    resized.connect(reset_view)
    mouse_exited.connect(func(): hovered_id = -1; queue_redraw())
    call_deferred("reset_view")

func configure(atlas_data: Dictionary) -> void:
    regions.clear()
    links.clear()
    settlements.clear()
    for item in atlas_data.get("regions", []): regions.append(item)
    for item in atlas_data.get("links", []): links.append(item)
    for item in atlas_data.get("settlements", []): settlements.append(item)
    queue_redraw()

func reset_view() -> void:
    zoom = minf(size.x / 1100.0, size.y / 650.0)
    if zoom <= 0.0: zoom = 1.0
    pan = (size - Vector2(1100, 650) * zoom) * 0.5
    queue_redraw()

func set_selected(id: int) -> void:
    if selected_id != id:
        selected_id = id
        queue_redraw()

func region_for_id(id: int) -> Dictionary:
    for item in regions:
        if int(item.get("id", -1)) == id: return item
    return {}

func settlement_for_territory(id: int) -> Dictionary:
    for item in settlements:
        if int(item.get("territory_id", -1)) == id: return item
    return {}

func region_at(local_position: Vector2) -> int:
    var point = (local_position - pan) / zoom
    for item in regions:
        if Geometry2D.is_point_in_polygon(point, item["polygon"]):
            return int(item["id"])
    return -1

func _gui_input(event: InputEvent) -> void:
    if event is InputEventMouseButton:
        if event.button_index in [MOUSE_BUTTON_MIDDLE, MOUSE_BUTTON_RIGHT]:
            dragging = event.pressed
            mouse_default_cursor_shape = Control.CURSOR_DRAG if dragging else Control.CURSOR_POINTING_HAND
            accept_event()
        elif event.pressed and event.button_index == MOUSE_BUTTON_LEFT:
            var id = region_at(event.position)
            if id >= 0: territory_selected.emit(id)
            accept_event()
        elif event.pressed and event.button_index in [MOUSE_BUTTON_WHEEL_UP, MOUSE_BUTTON_WHEEL_DOWN]:
            var multiplier = 1.13 if event.button_index == MOUSE_BUTTON_WHEEL_UP else 1.0 / 1.13
            var previous = zoom
            zoom = clampf(zoom * multiplier, 0.45, 2.5)
            pan = event.position - (event.position - pan) * zoom / previous
            queue_redraw()
            accept_event()
    elif event is InputEventMouseMotion:
        if dragging: pan += event.relative
        hovered_id = region_at(event.position)
        var hovered = region_for_id(hovered_id)
        var site = settlement_for_territory(hovered_id)
        var site_label = "%s · %s" % [site.get("name", ""), site.get("kind", "")] if not site.is_empty() else "chưa có địa điểm trung tâm"
        tooltip_text = "%s · %s · %s · Chọn để xem hồ sơ" % [hovered.get("name", "Bản đồ Atlas27"), hovered.get("group", "Kéo để di chuyển bản đồ"), site_label]
        queue_redraw()

func _draw() -> void:
    draw_rect(Rect2(Vector2.ZERO, size), INK)
    for x in range(0, int(size.x), 48): draw_line(Vector2(x, 0), Vector2(x, size.y), Color(0.25, 0.40, 0.43, 0.08), 1)
    for y in range(0, int(size.y), 48): draw_line(Vector2(0, y), Vector2(size.x, y), Color(0.25, 0.40, 0.43, 0.08), 1)
    draw_set_transform(pan, 0, Vector2(zoom, zoom))
    for item in regions:
        var polygon: PackedVector2Array = item["polygon"]
        var center: Vector2 = item["anchor"]
        var shadow := PackedVector2Array()
        for point in polygon: shadow.append(point + Vector2(3, 7))
        draw_colored_polygon(shadow, Color(0, 0, 0, 0.35))
        var group = str(item.get("group", ""))
        var fill = _region_color(group, bool(item.get("royal", false)))
        if int(item["id"]) == hovered_id: fill = fill.lightened(0.12)
        draw_colored_polygon(polygon, fill)
        var outline := PackedVector2Array(polygon)
        outline.append(polygon[0])
        draw_polyline(outline, GOLD if int(item["id"]) == selected_id else Color("70827a"), 2.5 if int(item["id"]) == selected_id else 1.1, true)
        var title = str(item.get("name", ""))
        _label(center + Vector2(0, -3), title, 12, PAPER, serif)
        _label(center + Vector2(0, 17), str(item.get("id", 0)).pad_zeros(2) + " · " + group.to_upper(), 7, Color("bdc9bb"), regular)
        if bool(item.get("royal", false)):
            draw_circle(center + Vector2(0, -28), 3, GOLD)
    for site in settlements:
        var territory_id := int(site.get("territory_id", -1))
        var position: Vector2 = site.get("position", Vector2.ZERO)
        var color := Color("71c1ae") if str(site.get("kind", "")) == "Cảng" else GOLD
        draw_circle(position, 4.0 if territory_id == selected_id else 3.0, color)
        if territory_id == hovered_id or territory_id == selected_id:
            draw_string(regular, position + Vector2(7, 4), str(site.get("name", "")), HORIZONTAL_ALIGNMENT_LEFT, -1, 8, PAPER)
    if show_routes:
        for link in links:
            var source = region_for_id(int(link.get("from", -1)))
            var target = region_for_id(int(link.get("to", -1)))
            if source.is_empty() or target.is_empty(): continue
            var a: Vector2 = source["anchor"]
            var b: Vector2 = target["anchor"]
            var mode = int(link.get("mode", 0))
            var route_color = Color("548f96") if mode == 1 else (GOLD if mode == 2 else (Color("8da8a5") if mode == 3 else Color("b3a77e")))
            draw_line(a, b, Color(0.03, 0.08, 0.09, 0.72), 5.0, true)
            if mode == 0 or int(link.get("direction", 0)) == 0:
                for step in range(0, 8, 2):
                    var start = a.lerp(b, float(step) / 8.0)
                    draw_line(start, a.lerp(b, float(step + 1) / 8.0), route_color, 1.8, true)
            else:
                draw_line(a, b, route_color, 1.8, true)
    draw_set_transform(Vector2.ZERO, 0, Vector2.ONE)
    draw_string(regular, Vector2(22, 26), "ATLAS VƯƠNG QUỐC · atlas-cartography-v1", HORIZONTAL_ALIGNMENT_LEFT, -1, 10, MUTED)
    draw_string(serif, Vector2(22, 52), "Hai mươi bảy miền đất", HORIZONTAL_ALIGNMENT_LEFT, -1, 23, PAPER)
    draw_string(regular, Vector2(22, size.y - 16), "Tọa độ bản đồ giả tưởng · map_unit · cuộn để thu phóng · kéo chuột giữa/phải để di chuyển", HORIZONTAL_ALIGNMENT_LEFT, -1, 9, MUTED)

func _region_color(group: String, royal: bool) -> Color:
    if royal: return Color("66543b")
    if group == "lưu vực": return Color("31554b")
    if group == "cao nguyên" or group == "biên bắc": return Color("454b40")
    if group == "duyên hải": return Color("31505a")
    if group == "đông lộ": return Color("4e5140")
    return Color("424a43")

func _label(at: Vector2, text: String, font_size: int, color: Color, font: Font) -> void:
    var width = font.get_string_size(text, HORIZONTAL_ALIGNMENT_LEFT, -1, font_size).x
    draw_string(font, at - Vector2(width * 0.5, 0), text, HORIZONTAL_ALIGNMENT_LEFT, -1, font_size, color)
