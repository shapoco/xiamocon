import cadquery as cq
from math import sqrt

# -- Parameters --
wall_t = 2
back_inner_t = 7

board_w = 100
board_h = 50
board_t = 1.6

inner_w = board_w + 1
inner_h = board_h + 1
inner_t = 12.5
back_t = inner_t + wall_t

case_w = inner_w + wall_t * 2
case_h = inner_h + wall_t * 2

tfc_hole_x = -board_w / 2 + 15
tfc_hole_w = 15
tfc_hole_t = 2

speaker_x = board_w / 2 - 19
speaker_y = board_h / 2 - 14

xiao_hole_w = 10
xiao_hole_t = 4
xiao_hole_z = wall_t + back_inner_t + 4.6
xiao_hole_x = -board_w / 2 + 21.5

disp_hole_w = 31
disp_hole_h = 31
disp_hole_y = 1

pad_center_x = 33
pad_center_y = -9
key_offset = 9
abxy_r = 4
abxy_hole_r = abxy_r + 0.5

dpad_h = 7
dpad_w = (key_offset + dpad_h / 2) * 2
dpad_hole_w = dpad_w + 1
dpad_hole_h = dpad_h + 1

key_flange_t = 0.5
key_t = wall_t + 2.5 + key_flange_t
key_top_offset = 0.3
key_bottom_hole_depth = 0.5
key_bottom_hole_d = 6.5

battery_w = 36
battery_h = 25

extport_w = 27
extport_h = 6

top_key_hole_w = 5
top_key_hole_t = inner_t - (back_inner_t + board_t)
top_key_w = top_key_hole_w - 0.5
top_key_t = top_key_hole_t - 0.5
top_key_flange_w = top_key_w + 3.5
func_sw_x = board_w / 2 - 7.5
power_sw_x = board_w / 2 - 30
volume_x = (func_sw_x + power_sw_x) / 2

# total_h = wall_t + back_inner_h + board_t


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
back = rr_extrude(case_w, case_h, 7, back_t)

# Inner cavity from z = wall_t
back = back.cut(
    rr_extrude(case_w - wall_t * 2, case_h - wall_t * 2, 4, inner_t).translate(
        (0, 0, wall_t)
    )
)


# Speaker hole through back
back = back.cut(rr_extrude(10, 4, 2, 99).translate((speaker_x, speaker_y, -1)))

# Battery retaining walls
battery_wall_t = back_inner_t - 1
back = (
    back.union(
        box_corner(
            -battery_w / 2 - 2,
            -inner_h / 2,
            wall_t,
            2,
            battery_h + wall_t + 1,
            battery_wall_t,
        )
    )
    .union(
        box_corner(
            battery_w / 2,
            -inner_h / 2,
            wall_t,
            2,
            battery_h + wall_t + 1,
            battery_wall_t,
        )
    )
    .union(
        box_corner(
            -battery_w / 2 + 2,
            -inner_h / 2 + battery_h + 1,
            wall_t,
            battery_w - 2,
            2,
            battery_wall_t,
        )
    )
)


# ---- TFC hole ----

tfc_block_h = wall_t + back_inner_t - tfc_hole_t

# Reinforcement block around TFC opening
back = back.union(
    box_corner(
        tfc_hole_x - tfc_hole_w / 2 - 2, -inner_h / 2, 0, tfc_hole_w + 4, 6, tfc_block_h
    )
)

# Cylindrical cutout
back = back.cut(cyl(tfc_hole_x, -board_h / 2 - 7, 0, 9, wall_t + back_inner_t))

# Slot cutout
back = back.cut(
    box_corner(
        tfc_hole_x - tfc_hole_w / 2, -board_h / 2 - 3, tfc_block_h, tfc_hole_w, 10, 2
    )
)


# Expansion port hole

back = back.cut(
    cq.Workplane("XY")
    .box(extport_w, extport_h, 99, centered=True)
    .translate((0, board_h / 2 - 14, 0))
)

# Power / Func switch clearance

top_key_hole = cq.Workplane("XY").box(
    top_key_hole_w, 100, top_key_hole_t, centered=[True, False, False]
)
top_key_hole = top_key_hole.union(
    cq.Workplane("XY").box(
        top_key_flange_w + 1, 2, top_key_hole_t, centered=[True, True, False]
    )
)
back = back.cut(
    top_key_hole.translate((func_sw_x, inner_h / 2, back_t - top_key_hole_t))
)
back = back.cut(
    top_key_hole.translate((power_sw_x, inner_h / 2, back_t - top_key_hole_t))
)

# Volume knob clearance

volume_hole_z = wall_t + back_inner_t + board_t
volume_hole_t = back_t - volume_hole_z
volume_hole = cyl(0, 5, 0, 6, 99)
volume_hole = volume_hole.union(
    cq.Workplane("XY")
    .box(10, 99, volume_hole_t, centered=[True, True, False])
    .translate((0, 0, 0))
)
volume_hole = volume_hole.translate((volume_x, inner_h / 2, volume_hole_z))
back = back.cut(volume_hole)
back = back.cut(cyl(volume_x, inner_h / 2 - 13, volume_hole_z, 15, 99))

# XIAO clearance hole

xiao_hole = cq.Workplane("XY").box(
    xiao_hole_w, 99, xiao_hole_t, centered=[True, False, True]
)
xiao_hole = xiao_hole.edges("|Y").fillet(1.5)
xiao_hole = xiao_hole.translate((xiao_hole_x, board_h / 2 - 6, xiao_hole_z))
back = back.cut(xiao_hole)

# Strap hole

back = back.union(
    cq.Workplane("XY")
    .box(8, 5, 5, centered=[True, False, False])
    .translate((board_w / 2 - 15, -inner_h / 2, wall_t))
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
    .edges(">Y and >Z")
    .fillet(3)
)

back = back.cut(strap_cutter.translate((board_w / 2 - 15, -case_h / 2 + 2, 2)))

# Screw pillars

pillar_h = wall_t + back_inner_t

screw_poses = [
    (-board_w / 2 + 4, -board_h / 2 + 4),
    (board_w / 2 - 4, -board_h / 2 + 4),
    (-board_w / 2 + 4, board_h / 2 - 4),
    (board_w / 2 - 4, board_h / 2 - 7),
]

for px, py in screw_poses:
    pillar = cyl(0, 0, 0, 3, pillar_h).cut(cyl(0, 0, wall_t, 2.8 / 2, 100))
    if px > 0:
        pillar = pillar.union(
            cq.Workplane("XY")
            .box(board_w / 2 - px - 1, 3, pillar_h, centered=[False, True, False])
            .translate((2, 0, 0))
        )
    if px < 0:
        pillar = pillar.union(
            cq.Workplane("XY")
            .box(board_w / 2 + px - 1, 3, pillar_h, centered=[False, True, False])
            .translate((2, 0, 0))
            .rotate((0, 0, 0), (0, 0, 1), 180)
        )
    if py > 0:
        pillar = pillar.union(
            cq.Workplane("XY")
            .box(3, board_h / 2 - py - 1, pillar_h, centered=[True, False, False])
            .translate((0, 2, 0))
        )
    if py < 0:
        pillar = pillar.union(
            cq.Workplane("XY")
            .box(3, board_h / 2 + py - 1, pillar_h, centered=[True, False, False])
            .translate((0, 2, 0))
            .rotate((0, 0, 0), (0, 0, 1), 180)
        )
    pillar = pillar.translate((px, py, 0))
    back = back.union(pillar)

# Grooves to prevent warping

back = back.edges("<Z").chamfer(0.5)

groove_w = sqrt(wall_t)

cutter = (
    cq.Workplane("XY")
    .box(groove_w, 99, groove_w, centered=[True, True, True])
    .rotate((0, 0, 0), (0, 1, 0), 45)
)
back = back.cut(cutter.translate((20, 0, 0)))
back = back.cut(cutter.translate((-20, 0, 0)))

cutter = (
    cq.Workplane("XY")
    .box(groove_w, inner_h, groove_w, centered=[True, True, True])
    .rotate((0, 0, 0), (0, 1, 0), 45)
)
back = back.cut(cutter.translate((20 + wall_t + groove_w, 0, wall_t)))
back = back.cut(cutter.translate((-20 - wall_t - groove_w, 0, wall_t)))

front = rr_extrude(case_w, case_h, 7, wall_t).translate((0, 0, back_t))

# display hole
disp_hole = (
    cq.Workplane("XY")
    .box(disp_hole_w, disp_hole_h, 99, centered=[True, True, False])
    .translate((0, disp_hole_y, 0))
)
front = front.cut(disp_hole)
front = front.edges(">Z and (not (<X or >X or >Y or <Y or %circle))").chamfer(1.5)

# Volume knob clearance in front plate
front = front.cut(volume_hole)

# ABXY button holes
abxy_hole = cyl(0, 0, 0, abxy_hole_r, 99)
front = front.cut(abxy_hole.translate((pad_center_x + key_offset, pad_center_y, 0)))
front = front.cut(abxy_hole.translate((pad_center_x - key_offset, pad_center_y, 0)))
front = front.cut(abxy_hole.translate((pad_center_x, pad_center_y + key_offset, 0)))
front = front.cut(abxy_hole.translate((pad_center_x, pad_center_y - key_offset, 0)))

# D-pad hole
dpad_hole = cq.Workplane("XY").box(
    dpad_hole_w, dpad_hole_h, 99, centered=[True, True, True]
)
dpad_hole = dpad_hole.union(dpad_hole.rotate((0, 0, 0), (0, 0, 1), 90))
dpad_hole = dpad_hole.edges("|Z").fillet(1)
front = front.cut(dpad_hole.translate((-pad_center_x, pad_center_y, 0)))

pillar_t = inner_t - back_inner_t - board_t - 1
led_pillar = cyl(0, 0, 0, 2, pillar_t)
led_pillar = led_pillar.union(
    cq.Workplane("XY")
    .polyline(
        [
            (0, 0, 0),
            (3, 0, 0),
            (0, 0, 3),
        ]
    )
    .close()
    .revolve(360, (0, 0, 0), (0, 0, 1))
)
led_pillar = led_pillar.rotate((0, 0, 0), (1, 0, 0), 180)
led_poses = [
    (15.5, 23.5),
    (-18.5, -20),
]
for lx, ly in led_poses:
    front = front.union(led_pillar.translate((lx, ly, back_t)))
    front = front.cut(cyl(lx, ly, 0, 1.25, 99))

front = front.cut(cyl(xiao_hole_x - 5.5, board_h / 2 - 2, 0, 1.25, 99))
front = front.cut(cyl(xiao_hole_x + 5.5, board_h / 2 - 2, 0, 1.25, 99))

cutter = (
    cq.Workplane("XY")
    .box(10, 5, 5, centered=[True, False, False])
    .translate((15.5, case_h / 2 - wall_t, back_t - 5))
)
front = front.cut(cutter)

speaker_hole_h = 2
speaker_hole = cq.Workplane("XY").box(20, speaker_hole_h, 99, centered=True)
speaker_hole = speaker_hole.edges("|Z").fillet(speaker_hole_h / 2 - 0.1)
front = front.cut(speaker_hole.translate((pad_center_x, speaker_y, 0)))

front = front.edges(">Z and (<X or %circle)").chamfer(0.5)

front = front.cut(xiao_hole)

screw_hole = cyl(0, 0, 0, 3.5 / 2, 99)
screw_hole = screw_hole.union(
    cq.Workplane("XY")
    .polyline(
        [
            (0, 0, 0),
            (3.5, 0, 0),
            (0, 0, 3.5),
        ]
    )
    .close()
    .revolve(360, (0, 0, 0), (0, 0, 1))
)
screw_hole = screw_hole.rotate((0, 0, 0), (1, 0, 0), 180)

pillar_t = inner_t - back_inner_t - board_t - 0.2
screw_pillar = cyl(0, 0, 0, 3, pillar_t)
screw_pillar = screw_pillar.union(
    cq.Workplane("XY")
    .polyline(
        [
            (0, 0, 0),
            (4, 0, 0),
            (0, 0, 4),
        ]
    )
    .close()
    .revolve(360, (0, 0, 0), (0, 0, 1))
)
screw_pillar = screw_pillar.rotate((0, 0, 0), (1, 0, 0), 180)

for px, py in screw_poses:
    front = front.union(
        screw_pillar.translate((px, py, back_t))
    )  # Screw clearance holes in front plate
    front = front.cut(screw_hole.translate((px, py, back_t + wall_t)))

# guard rails
guard_h = 2
guard_offset = wall_t + guard_h + 0.2
guard = cq.Workplane("XY").box(20, guard_h, 1, centered=[True, False, False])
guard = guard.rotate((0, 0, 0), (0, 1, 0), 180)
front = front.union(guard.translate((0, case_h / 2 - guard_offset, back_t)))
guard = guard.rotate((0, 0, 0), (0, 0, 1), 180)
front = front.union(guard.translate((0, -case_h / 2 + guard_offset, back_t)))
guard = guard.rotate((0, 0, 0), (0, 0, 1), 90)
front = front.union(guard.translate((case_w / 2 - guard_offset, 0, back_t)))
guard = guard.rotate((0, 0, 0), (0, 0, 1), 180)
front = front.union(guard.translate((-case_w / 2 + guard_offset, 0, back_t)))

key_bottom_hole = cyl(0, 0, -10, key_bottom_hole_d / 2, key_bottom_hole_depth + 10)
key_bottom_hole = key_bottom_hole.edges(">Z").chamfer(key_bottom_hole_depth - 0.1)

# 十字キー本体の作成
verts1 = [
    (-dpad_w / 2, 0),
    (-dpad_w / 2 + (1 + key_top_offset), key_t),
    (dpad_w / 2 - (1 + key_top_offset), key_t),
    (dpad_w / 2, 0),
]
verts2 = [
    (-dpad_h / 2, 0),
    (-dpad_h / 2 + key_top_offset, key_t),
    (dpad_h / 2 - key_top_offset, key_t),
    (dpad_h / 2, 0),
]
dpad_key = cq.Workplane("YZ").polyline(verts1).close().extrude(dpad_h / 2, both=True)
dpad_key = dpad_key.intersect(
    cq.Workplane("XZ").polyline(verts2).close().extrude(dpad_w / 2, both=True)
)
dpad_key = dpad_key.union(dpad_key.rotate((0, 0, 0), (0, 0, 1), 90))
dpad_key = dpad_key.edges("not (>Z or <Z)").fillet(1)

# 十字キーのフランジ
dpad_key_flange = (
    cq.Workplane("XY")
    .box(
        dpad_h + 8,
        dpad_h + 8,
        key_flange_t,
        centered=(True, True, False),
    )
    .rotate((0, 0, 0), (0, 0, 1), 45)
)
dpad_key_flange = dpad_key_flange.union(dpad_key_flange.rotate((0, 0, 0), (0, 0, 1), 90))
dpad_key = dpad_key.union(dpad_key_flange)

# 十字キートップのくぼみ
dpad_key = dpad_key.cut(
    cq.Workplane("XY").sphere(150).translate((0, 0, key_t + 150 - 0.5))
)

# 十字キートップの丸め
dpad_key = dpad_key.faces(">Z").fillet(0.5)

# 十字キー裏の穴
for angle in [0, 90, 180, 270]:
    dpad_key = dpad_key.cut(
        key_bottom_hole.rotate((0, 0, 0), (0, 1, 0), -10)
        .translate((key_offset - 0.5, 0, 0))
        .rotate((0, 0, 0), (0, 0, 1), angle)
    )

# Pivot of bottom of D-pad key
pivot_t = 2.5
pivot_r = 2
pivot = cyl(0, 0, 0, pivot_r, pivot_t + 1)
pivot = pivot.edges("<Z").chamfer(pivot_r - 1)
dpad_key = dpad_key.union(pivot.translate((0, 0, -pivot_t)))

# ABXYキー本体の作成
verts = [
    (0, 0),
    (abxy_r, 0),
    (abxy_r - key_top_offset, key_t),
    (0, key_t),
]
abxy_key = (
    cq.Workplane("YZ")
    .polyline(verts)
    .close()
    .revolve(360, axisStart=(0, 0, 0), axisEnd=(0, 1, 0))
)

# ABXYキーのフランジ
abxy_key = abxy_key.union(
    cq.Workplane("XY").cylinder(
        key_flange_t,
        abxy_r + 1,
        centered=(True, True, False),
    )
)

# 十字キートップのくぼみ
abxy_key = abxy_key.cut(
    cq.Workplane("XY").sphere(10).translate((0, 0, key_t + 10 - 0.4))
)

# ABXYキートップの丸め
abxy_key = abxy_key.faces(">Z").fillet(0.5)

# ABXYキー裏の穴
abxy_key = abxy_key.cut(key_bottom_hole)

# Power/Func Key
top_key = cq.Workplane("XY").box(
    top_key_w, 8.5, top_key_t, centered=[True, False, False]
).translate((0, -8, 0))
top_key = top_key .union(
    cq.Workplane("XY").box(top_key_flange_w, 1, top_key_t, centered=[True , False, False]).translate((0, -wall_t, 0))
)
top_key = top_key .cut(
    cq.Workplane("XY").box(top_key_w, 3, 2, centered=[True , False, False]).translate((0, -6, 0))
)

power_key = top_key.cut(
    cq.Workplane("XY").box(10, 99, 99, centered=[False, True, True]).translate((-10 - top_key_w / 2, 0, 0))
)
func_key = top_key.cut(
    cyl(6, -9, 0, 6, 99)
)

show_object(back, name="back_box")
show_object(power_key.translate((power_sw_x, case_h / 2, back_t - top_key_t - 0.25)), name="power_key")
show_object(func_key.translate((func_sw_x, case_h / 2, back_t - top_key_t - 0.25)), name="func_key")

#show_object(front, name="front_box")
#show_object(
#    dpad_key.translate((-pad_center_x, pad_center_y, back_t - key_flange_t)),
#    name="dpad_key",
#)
#show_object(
#    abxy_key.translate(
#        (pad_center_x - key_offset, pad_center_y, back_t - key_flange_t)
#    ),
#    name="y_key",
#)
#show_object(
#    abxy_key.translate(
#        (pad_center_x + key_offset, pad_center_y, back_t - key_flange_t)
#    ),
#    name="a_key",
#)
#show_object(
#    abxy_key.translate(
#        (pad_center_x, pad_center_y - key_offset, back_t - key_flange_t)
#    ),
#    name="b_key",
#)
#show_object(
#    abxy_key.translate(
#        (pad_center_x, pad_center_y + key_offset, back_t - key_flange_t)
#    ),
#    name="x_key",
#)

front = front.rotate((0, 0, back_t), (1, 0, back_t), 180).translate(
    (0, 0, -back_t + wall_t)
)

dpad_key = dpad_key.rotate((0, 0, 0), (0, 0, 1), 45).rotate(
    (0, -dpad_w / 2, 0), (1, -dpad_w / 2, 0), 90
)
abxy_key = abxy_key.rotate(
    (0, -abxy_r - key_flange_t, 0), (1, -abxy_r - key_flange_t, 0), 90
)

front.export("front.stl")
front.export("front.step")
back.export("back.stl")
back.export("back.step")
dpad_key.export("dpad_key.stl")
dpad_key.export("dpad_key.step")
abxy_key.export("abxy_key.stl")
abxy_key.export("abxy_key.step")
