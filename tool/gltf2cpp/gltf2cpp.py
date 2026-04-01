#!/usr/bin/env python3
"""gltf2cpp - Convert glTF 2.0 files to C++ header files for xmc."""

import argparse
import io
import os
import struct
import sys
from pathlib import Path

from PIL import Image
import pygltflib


GLTF_PRIMITIVE_MODES = {
    0: "xmc::PrimitiveMode::POINTS",
    1: "xmc::PrimitiveMode::LINES",
    2: "xmc::PrimitiveMode::LINE_LOOP",
    3: "xmc::PrimitiveMode::LINE_STRIP",
    4: "xmc::PrimitiveMode::TRIANGLES",
    5: "xmc::PrimitiveMode::TRIANGLE_STRIP",
    6: "xmc::PrimitiveMode::TRIANGLE_FAN",
}

COMPONENT_SIZES = {
    5120: 1,   # BYTE
    5121: 1,   # UNSIGNED_BYTE
    5122: 2,   # SHORT
    5123: 2,   # UNSIGNED_SHORT
    5125: 4,   # UNSIGNED_INT
    5126: 4,   # FLOAT
}

COMPONENT_FMT = {
    5120: "b",
    5121: "B",
    5122: "h",
    5123: "H",
    5125: "I",
    5126: "f",
}

TYPE_COMPONENTS = {
    "SCALAR": 1,
    "VEC2": 2,
    "VEC3": 3,
    "VEC4": 4,
    "MAT2": 4,
    "MAT3": 9,
    "MAT4": 16,
}


def read_accessor(gltf, accessor_index):
    """Read accessor data as a list of tuples."""
    acc = gltf.accessors[accessor_index]
    bv = gltf.bufferViews[acc.bufferView]
    blob = gltf.binary_blob()
    if blob is None:
        raise RuntimeError("No binary data available")

    offset = (bv.byteOffset or 0) + (acc.byteOffset or 0)
    n_comp = TYPE_COMPONENTS[acc.type]
    comp_size = COMPONENT_SIZES[acc.componentType]
    elem_size = n_comp * comp_size
    stride = bv.byteStride or elem_size
    fmt = COMPONENT_FMT[acc.componentType]

    result = []
    for i in range(acc.count):
        off = offset + i * stride
        values = struct.unpack_from(f"<{n_comp}{fmt}", blob, off)
        result.append(values)
    return result, acc


def load_image(gltf, texture_index, gltf_dir):
    """Load a texture image from glTF (handles bufferView and URI)."""
    tex = gltf.textures[texture_index]
    img_info = gltf.images[tex.source]

    if img_info.bufferView is not None:
        bv = gltf.bufferViews[img_info.bufferView]
        blob = gltf.binary_blob()
        offset = bv.byteOffset or 0
        data = blob[offset : offset + bv.byteLength]
        return Image.open(io.BytesIO(data))
    elif img_info.uri:
        if img_info.uri.startswith("data:"):
            import base64

            _, b64 = img_info.uri.split(",", 1)
            return Image.open(io.BytesIO(base64.b64decode(b64)))
        else:
            return Image.open(os.path.join(gltf_dir, img_info.uri))
    raise RuntimeError(f"Cannot load texture {texture_index}")


def to_argb4444(image, dither=False):
    """Convert PIL Image to ARGB4444 pixel array (list of uint16)."""
    image = image.convert("RGBA")
    w, h = image.size
    raw = image.tobytes()

    if not dither:
        pixels = []
        for i in range(0, len(raw), 4):
            r, g, b, a = raw[i], raw[i + 1], raw[i + 2], raw[i + 3]
            pixels.append(((a >> 4) << 12) | ((r >> 4) << 8) | ((g >> 4) << 4) | (b >> 4))
        return w, h, pixels

    # Floyd-Steinberg error diffusion dithering
    buf = [[0.0] * 4 for _ in range(w * h)]
    for i in range(0, len(raw), 4):
        idx = i // 4
        buf[idx] = [float(raw[i]), float(raw[i + 1]), float(raw[i + 2]), float(raw[i + 3])]

    pixels = []
    for y in range(h):
        for x in range(w):
            idx = y * w + x
            old = buf[idx]
            # Quantize to 4-bit per channel
            new = [max(0.0, min(255.0, c)) for c in old]
            q = [(int(c + 0.5) >> 4) & 0xF for c in new]
            # Expanded quantized values
            eq = [v * 17 for v in q]  # 0xF -> 255, 0x0 -> 0
            # Quantization error
            err = [new[ch] - eq[ch] for ch in range(4)]
            pixels.append((q[3] << 12) | (q[0] << 8) | (q[1] << 4) | q[2])
            # Distribute error to neighbors (Floyd-Steinberg)
            for dx, dy, weight in [(1, 0, 7 / 16), (-1, 1, 3 / 16), (0, 1, 5 / 16), (1, 1, 1 / 16)]:
                nx, ny = x + dx, y + dy
                if 0 <= nx < w and 0 <= ny < h:
                    ni = ny * w + nx
                    for ch in range(4):
                        buf[ni][ch] += err[ch] * weight

    return w, h, pixels


def fmt_float(f):
    """Format a float for C++ output."""
    s = f"{f:.6f}".rstrip("0")
    if s.endswith("."):
        s += "0"
    return s + "f"


def sanitize_id(name):
    """Sanitize a string for use as a C++ identifier."""
    result = ""
    for c in name:
        result += c if (c.isalnum() or c == "_") else "_"
    if result and result[0].isdigit():
        result = "_" + result
    return result


def generate(gltf, gltf_dir, prefix, dither=False):
    """Generate C++ header content from glTF data."""
    out = []
    w = out.append

    w("#pragma once")
    w("")
    w('#include "xmc/gfx/3d/scene3d.hpp"')
    w("")
    w("namespace {")

    # ── Materials ──
    mat_vars = {}
    materials = gltf.materials or []
    for mi, mat in enumerate(materials):
        w("")
        mn = f"{prefix}_mat{mi}"
        pbr = mat.pbrMetallicRoughness

        # Base color texture
        tex_var = None
        has_tex = pbr and pbr.baseColorTexture and pbr.baseColorTexture.index is not None
        if has_tex:
            try:
                img = load_image(gltf, pbr.baseColorTexture.index, gltf_dir)
                tw, th, pixels = to_argb4444(img, dither=dither)

                dn = f"{mn}_colorTextureData"
                w(f"const uint16_t {dn}[] = {{")
                for i in range(0, len(pixels), 8):
                    chunk = pixels[i : i + 8]
                    w("    " + ", ".join(f"0x{v:04X}" for v in chunk) + ",")
                w("};")
                w("")

                tex_var = f"{mn}_colorTexture"
                w(f"const xmc::Sprite {tex_var} = xmc::createSprite4444(")
                w(f"    {tw}, {th}, 0,")
                w(f"    (uint16_t *){dn}, false")
                w(");")
                w("")
            except Exception as e:
                print(f"Warning: Could not load texture for material {mi}: {e}", file=sys.stderr)

        # Base color factor
        bc = [1.0, 1.0, 1.0, 1.0]
        if pbr and pbr.baseColorFactor:
            bc = list(pbr.baseColorFactor)
        while len(bc) < 4:
            bc.append(1.0)

        fn = f"{mn}_create"
        w(f"xmc::Material3D {fn}() {{")
        w("  xmc::Material3DClass mat;")
        w(f"  mat.baseColor = {{{fmt_float(bc[0])}, {fmt_float(bc[1])}, {fmt_float(bc[2])}, {fmt_float(bc[3])}}};")
        if tex_var:
            w(f"  mat.colorTexture = {tex_var};")
        if mat.doubleSided:
            w("  mat.doubleSided = true;")
        w("  return std::make_shared<xmc::Material3DClass>(mat);")
        w("}")
        w("")
        w(f"const xmc::Material3D {mn} = {fn}();")

        mat_vars[mi] = mn

    # ── Meshes ──
    meshes = gltf.meshes or []
    for mi, mesh in enumerate(meshes):
        mesh_name = f"{prefix}_mesh{mi}"
        prim_vars = []

        for pi, prim in enumerate(mesh.primitives):
            pn = f"{mesh_name}_prim{pi}"
            attrs = prim.attributes
            mode = prim.mode if prim.mode is not None else 4
            mode_str = GLTF_PRIMITIVE_MODES.get(mode, "xmc::PrimitiveMode::TRIANGLES")

            # ── POSITION ──
            pos_var = "nullptr"
            if attrs.POSITION is not None:
                data, _ = read_accessor(gltf, attrs.POSITION)
                dn = f"{pn}_posData"
                vn = f"{pn}_pos"
                w("")
                w(f"const float {dn}[] = {{")
                for v in data:
                    w(f"    {fmt_float(v[0])}, {fmt_float(v[1])}, {fmt_float(v[2])},")
                w("};")
                w("")
                w(f"const xmc::Vec3Buffer {vn} = xmc::createVec3Buffer(")
                w(f"    (xmc::vec3 *){dn}, {len(data)}, false")
                w(");")
                pos_var = f"(xmc::Vec3Buffer){vn}"

            # ── NORMAL ──
            norm_var = "nullptr"
            if attrs.NORMAL is not None:
                data, _ = read_accessor(gltf, attrs.NORMAL)
                dn = f"{pn}_normData"
                vn = f"{pn}_norm"
                w("")
                w(f"const float {dn}[] = {{")
                for v in data:
                    w(f"    {fmt_float(v[0])}, {fmt_float(v[1])}, {fmt_float(v[2])},")
                w("};")
                w("")
                w(f"const xmc::Vec3Buffer {vn} = xmc::createVec3Buffer(")
                w(f"    (xmc::vec3 *){dn}, {len(data)}, false")
                w(");")
                norm_var = f"(xmc::Vec3Buffer){vn}"

            # ── COLOR_0 ──
            col_var = "nullptr"
            if attrs.COLOR_0 is not None:
                data, acc = read_accessor(gltf, attrs.COLOR_0)
                ct = acc.componentType
                nc = TYPE_COMPONENTS[acc.type]
                dn = f"{pn}_colData"
                vn = f"{pn}_col"
                w("")
                w(f"const float {dn}[] = {{")
                for v in data:
                    if ct == 5126:  # FLOAT
                        r, g, b = v[0], v[1], v[2]
                        a = v[3] if nc >= 4 else 1.0
                    elif ct == 5121:  # UNSIGNED_BYTE
                        r, g, b = v[0] / 255.0, v[1] / 255.0, v[2] / 255.0
                        a = v[3] / 255.0 if nc >= 4 else 1.0
                    elif ct == 5123:  # UNSIGNED_SHORT
                        r, g, b = v[0] / 65535.0, v[1] / 65535.0, v[2] / 65535.0
                        a = v[3] / 65535.0 if nc >= 4 else 1.0
                    else:
                        r, g, b, a = 1.0, 1.0, 1.0, 1.0
                    w(f"    {fmt_float(r)}, {fmt_float(g)}, {fmt_float(b)}, {fmt_float(a)},")
                w("};")
                w("")
                w(f"const xmc::ColorBuffer {vn} = xmc::createColorBuffer(")
                w(f"    (xmc::colorf *){dn}, {len(data)}, false")
                w(");")
                col_var = f"(xmc::ColorBuffer){vn}"

            # ── TEXCOORD_0 ──
            uv_var = "nullptr"
            if attrs.TEXCOORD_0 is not None:
                data, _ = read_accessor(gltf, attrs.TEXCOORD_0)
                dn = f"{pn}_uvData"
                vn = f"{pn}_uv"
                w("")
                w(f"const float {dn}[] = {{")
                for v in data:
                    w(f"    {fmt_float(v[0])}, {fmt_float(v[1])},")
                w("};")
                w("")
                w(f"const xmc::Vec2Buffer {vn} = xmc::createVec2Buffer(")
                w(f"    (xmc::vec2 *){dn}, {len(data)}, false")
                w(");")
                uv_var = f"(xmc::Vec2Buffer){vn}"

            # ── Indices ──
            idx_var = "nullptr"
            if prim.indices is not None:
                data, acc = read_accessor(gltf, prim.indices)
                flat = [int(v[0]) for v in data]
                if acc.componentType == 5125:  # UNSIGNED_INT
                    over = [v for v in flat if v > 65535]
                    if over:
                        print(
                            f"Warning: {len(over)} indices exceed uint16 range in primitive {pi} of mesh {mi}",
                            file=sys.stderr,
                        )
                dn = f"{pn}_idxData"
                vn = f"{pn}_idx"
                w("")
                w(f"const uint16_t {dn}[] = {{")
                for i in range(0, len(flat), 16):
                    chunk = flat[i : i + 16]
                    w("    " + ", ".join(str(v & 0xFFFF) for v in chunk) + ",")
                w("};")
                w("")
                w(f"const xmc::IndexBuffer {vn} = xmc::createIndexBuffer(")
                w(f"    (uint16_t *){dn}, {len(data)}, false")
                w(");")
                idx_var = f"(xmc::IndexBuffer){vn}"

            # ── Material reference ──
            mat_var = "nullptr"
            if prim.material is not None and prim.material in mat_vars:
                mat_var = f"(xmc::Material3D){mat_vars[prim.material]}"

            # ── Primitive ──
            fn = f"{pn}_create"
            w("")
            w(f"xmc::Primitive3D {fn}() {{")
            w("  return xmc::createPrimitive3D(")
            w(f"      {mode_str},")
            w(f"      {pos_var},")
            w(f"      {norm_var},")
            w(f"      {col_var},")
            w(f"      {uv_var},")
            w(f"      {idx_var},")
            w(f"      {mat_var}")
            w("  );")
            w("}")
            w("")
            w(f"const xmc::Primitive3D {pn} = {fn}();")

            prim_vars.append(pn)

        # ── Mesh ──
        fn = f"{mesh_name}_create"
        w("")
        w(f"xmc::Mesh3D {fn}() {{")
        prims = ", ".join(prim_vars)
        w(f"  std::vector<xmc::Primitive3D> prims = {{{prims}}};")
        w("  return xmc::createMesh3D(std::move(prims));")
        w("}")
        w("")
        w(f"const xmc::Mesh3D {mesh_name} = {fn}();")

    # ── Nodes ──
    mesh_vars = {mi: f"{prefix}_mesh{mi}" for mi in range(len(meshes))}
    node_vars = {}
    nodes = gltf.nodes or []
    for ni, node in enumerate(nodes):
        nn = f"{prefix}_node{ni}"
        node_vars[ni] = nn

    # Generate nodes in reverse dependency order is not needed since
    # we use create functions. Generate all nodes, then wire children.
    for ni, node in enumerate(nodes):
        nn = node_vars[ni]
        fn = f"{nn}_create"

        # ── Transform ──
        has_matrix = node.matrix is not None and list(node.matrix) != [
            1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1
        ]
        has_trs = (
            (node.translation is not None and list(node.translation) != [0, 0, 0])
            or (node.rotation is not None and list(node.rotation) != [0, 0, 0, 1])
            or (node.scale is not None and list(node.scale) != [1, 1, 1])
        )

        # Mesh reference
        mesh_ref = "nullptr"
        if node.mesh is not None and node.mesh in mesh_vars:
            mesh_ref = mesh_vars[node.mesh]

        w("")
        w(f"xmc::Node3D {fn}() {{")

        if has_matrix:
            mtx = list(node.matrix)
            w("  xmc::mat4 t;")
            for i in range(16):
                if mtx[i] != 0.0:
                    w(f"  t.m[{i}] = {fmt_float(mtx[i])};")
        elif has_trs:
            w("  xmc::mat4 t = xmc::mat4::identity();")
            if node.scale is not None and list(node.scale) != [1, 1, 1]:
                s = list(node.scale)
                w(f"  t.scale(xmc::vec3({fmt_float(s[0])}, {fmt_float(s[1])}, {fmt_float(s[2])}));")
            if node.rotation is not None and list(node.rotation) != [0, 0, 0, 1]:
                r = list(node.rotation)
                w(f"  t.rotate(xmc::quat({fmt_float(r[3])}, {fmt_float(r[0])}, {fmt_float(r[1])}, {fmt_float(r[2])}));")
            if node.translation is not None and list(node.translation) != [0, 0, 0]:
                tr = list(node.translation)
                w(f"  t.translate({fmt_float(tr[0])}, {fmt_float(tr[1])}, {fmt_float(tr[2])});")

        if has_matrix or has_trs:
            w(f"  xmc::Node3D node = xmc::createNode3D({mesh_ref}, t);")
        else:
            w(f"  xmc::Node3D node = xmc::createNode3D({mesh_ref});")

        # Children
        children = node.children or []
        for ci in children:
            if ci in node_vars:
                w(f"  node->children.push_back({node_vars[ci]});")

        w("  return node;")
        w("}")
        w("")
        w(f"const xmc::Node3D {nn} = {fn}();")

    # ── Scenes ──
    scenes = gltf.scenes or []
    scene_vars = {}
    for si, scene in enumerate(scenes):
        sn = f"{prefix}_scene{si}"
        scene_vars[si] = sn
        fn = f"{sn}_create"

        w("")
        w(f"xmc::Scene3D {fn}() {{")
        w("  xmc::Scene3D scene = xmc::createScene3D();")
        root_nodes = scene.nodes or []
        for ni in root_nodes:
            if ni in node_vars:
                w(f"  scene->addNode({node_vars[ni]});")
        w("  return scene;")
        w("}")
        w("")
        w(f"const xmc::Scene3D {sn} = {fn}();")

    # ── Default scene alias ──
    default_scene_idx = gltf.scene if gltf.scene is not None else (0 if scenes else None)
    if default_scene_idx is not None and default_scene_idx in scene_vars:
        w("")
        w(f"const xmc::Scene3D {prefix} = {scene_vars[default_scene_idx]};")

    w("")
    w("}  // namespace")
    w("")
    return "\n".join(out)


def main():
    parser = argparse.ArgumentParser(
        description="Convert glTF 2.0 files to C++ header files for xmc"
    )
    parser.add_argument("input", help="Input glTF file (.gltf or .glb)")
    parser.add_argument("output", help="Output C++ header file (.hpp)")
    parser.add_argument(
        "--prefix",
        "-p",
        default=None,
        help="Prefix for C++ identifiers (default: derived from input filename)",
    )
    parser.add_argument(
        "--dither",
        action="store_true",
        default=False,
        help="Apply Floyd-Steinberg error diffusion dithering when converting textures to ARGB4444",
    )
    args = parser.parse_args()

    input_path = os.path.abspath(args.input)
    output_path = os.path.abspath(args.output)
    gltf_dir = os.path.dirname(input_path)

    prefix = args.prefix if args.prefix else sanitize_id(Path(args.input).stem)

    gltf = pygltflib.GLTF2().load(input_path)
    if gltf.binary_blob() is None:
        gltf.convert_buffers(pygltflib.BufferFormat.BINARYBLOB)

    content = generate(gltf, gltf_dir, prefix, dither=args.dither)

    os.makedirs(os.path.dirname(output_path) or ".", exist_ok=True)
    with open(output_path, "w") as f:
        f.write(content)

    print(f"Generated: {output_path}")


if __name__ == "__main__":
    main()