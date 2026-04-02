import cadquery as cq

# -- Parameters --
wall_t = 2
back_inner_h = 6

board_w = 100
board_h = 50
board_t = 1.6

case_w = board_w + 1 * 2
case_h = board_h + 1 * 2

tfc_hole_x = -board_w / 2 + 15
tfc_hole_w = 15
tfc_hole_h = 2

speaker_x = board_w / 2 - 19
speaker_y = board_h / 2 - 14


battery_w = 36
battery_h = 25

func_sw_x = board_w / 2 - 7.5
power_sw_x = board_w / 2 - 30
volume_x = (func_sw_x + power_sw_x) / 2

total_h = wall_t + back_inner_h + board_t


# -- Helpers --


def rr_extrude(w, h, r, d):
    """Rounded-rect extrusion centered in XY, from z=0 to z=d."""
    return (
        cq.Workplane("XY")
        .sketch()
        .rect(w, h)
        .vertices()
        .fillet(r)
        .finalize()
        .extrude(d)
    )


def box_corner(x, y, z, sx, sy, sz):
    """Box with its min corner at (x, y, z)."""
    return (
        cq.Workplane("XY")
        .box(sx, sy, sz)
        .translate((x + sx / 2, y + sy / 2, z + sz / 2))
    )


def box_center(x, y, z, sx, sy, sz):
    """Box centered at (x, y, z)."""
    return cq.Workplane("XY").box(sx, sy, sz).translate((x, y, z))


def cyl(x, y, z, r, h):
    """Cylinder at (x, y), from z to z+h."""
    return cq.Workplane("XY").circle(r).extrude(h).translate((x, y, z))


# ---- back_box_base ----

# Outer shell
result = rr_extrude(case_w, case_h, 6, total_h)

result = result.edges(">Z").chamfer(0.5)

# Inner cavity from z = wall_t
result = result.cut(
    rr_extrude(case_w - wall_t * 2, case_h - wall_t * 2, 4, total_h).translate(
        (0, 0, wall_t)
    )
)

# Board ledge from z = wall_t + back_inner_h
result = result.cut(
    rr_extrude(case_w - 1.6, case_h - 1.6, 5, total_h).translate(
        (0, 0, wall_t + back_inner_h)
    )
)

# Speaker hole through back
result = result.cut(
    rr_extrude(10, 4, 2, total_h + 2).translate((speaker_x, speaker_y, -1))
)

# Diode clearance (top edge)
result = result.cut(box_center(0, board_h / 2 - 1, wall_t + back_inner_h, 20, 2, 2))

# Volume-foot clearance
result = result.cut(
    box_center(volume_x, board_h / 2 - 1, wall_t + back_inner_h, 13, 2, 2)
)

# Battery clearance (bottom edge)
result = result.cut(
    box_center(
        0, -board_h / 2 + 1, wall_t + back_inner_h, battery_w, 2, back_inner_h * 2
    )
)

# Battery retaining walls
bw_h = back_inner_h - 1
result = (
    result.union(
        box_corner(-battery_w / 2 - 2, -board_h / 2 + 1, wall_t, 2, battery_h + 2, bw_h)
    )
    .union(box_corner(battery_w / 2, -board_h / 2 + 1, wall_t, 2, battery_h + 2, bw_h))
    .union(
        box_corner(
            -battery_w / 2 + 2,
            -board_h / 2 + battery_h + 1,
            wall_t,
            battery_w - 2,
            2,
            bw_h,
        )
    )
)


# ---- TFC hole ----

tfc_block_h = wall_t + back_inner_h - tfc_hole_h

# Reinforcement block around TFC opening
result = result.union(
    box_corner(
        tfc_hole_x - tfc_hole_w / 2 - 2, -board_h / 2, 0, tfc_hole_w + 4, 5, tfc_block_h
    )
)

# Cylindrical cutout
result = result.cut(cyl(tfc_hole_x, -board_h / 2 - 7, -1, 9, 100))

# Slot cutout
result = result.cut(
    box_corner(
        tfc_hole_x - tfc_hole_w / 2, -board_h / 2 - 3, tfc_block_h, tfc_hole_w, 10, 100
    )
)


# Expansion port hole

result = result.cut(
    cq.Workplane("XY").box(26, 6, 99, centered=True).translate((0, board_h / 2 - 14, 0))
)

# Power / Func switch clearance

result = result.cut(
    cq.Workplane("XY")
    .box(5, 10, 99, centered=[True, False, False])
    .translate((func_sw_x, board_h / 2, wall_t + back_inner_h))
)

result = result.cut(
    cq.Workplane("XY")
    .box(5, 10, 99, centered=[True, False, False])
    .translate((power_sw_x, board_h / 2, wall_t + back_inner_h))
)

# Strap hole

result = result.union(
    cq.Workplane("XY")
    .box(8, 5, 5, centered=[True, False, False])
    .translate((board_w / 2 - 15, -board_h / 2, wall_t))
)

strap_cutter = (
    cq.Workplane("XY")
    .box(4, 10, 3, centered=[True, False, False])
    .translate((0, -7, 0))
    .union(
        cq.Workplane("XY")
        .box(4, 3, 10, centered=[True, False, False])
        .translate((0, 0, -7))
    )
    .edges(">Y and >Z").fillet(3)
)

result = result.cut(strap_cutter.translate((board_w / 2 - 15, -case_h / 2 + 2, 2)))

# ---- Screw pillars ----

pillar_h = wall_t + back_inner_h

for px, py in [
    (-board_w / 2 + 4, -board_h / 2 + 4),
    (board_w / 2 - 4, -board_h / 2 + 4),
    (-board_w / 2 + 4, board_h / 2 - 4),
    (board_w / 2 - 4, board_h / 2 - 7),
]:
    pillar = (
        cyl(0, 0, 0, 3, pillar_h)
        .cut(cyl(0, 0, wall_t, 1.25, 100))
        .translate((px, py, 0))
    )
    result = result.union(pillar)

result = result.union(
    cq.Workplane("XY")
    .box(2, 4, pillar_h, centered=[True, True, False])
    .translate((board_w / 2 - 1, board_h / 2 - 7, 0))
)

result = result.edges("<Z").chamfer(0.5)

show_object(result, name="back_box")

result.export("simple.stl")
result.export("simple.step")
