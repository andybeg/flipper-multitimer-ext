# Flipper E-Ink Enclosure Parts

Parametric OpenSCAD models for mounting a 4.2" e-paper module with Flipper Zero.

## Files

- `eink_screen_holder.scad` — angled screen holder with slide-in grooves and bottom mount tabs.
- `eink_screen_holder.stl` — exported holder STL.
- `adapter_pcb_box.scad` — parametric source for the adapter box and side wings.
- `adapter_pcb_box.stl` — main box body with wing sockets (no wings attached).
- `adapter_pcb_box_wing.stl` — side mount wing (print **two** copies; mirror one for the opposite side; insert from the top into the box sockets).
- `adapter_pcb_box_lid.stl` — optional lid insert.

## Before Printing

Measure your exact hardware and tune values in the `.scad` files.

`eink_screen_holder.scad`:

- `screen_board_w`, `screen_board_h` — WeAct e-paper PCB size (`screen_board_w` is derived from `mount_span_w`).
- `mount_span_w` — holder width and pin-to-pin span (96 mm).
- `holder_ratio` — how much of the board height the holder supports.
- `slot_gap`, `slot_wall`, `slot_lip` — slide-in groove dimensions.
- `tab_*` — bottom tabs for attaching the holder to the adapter box.

`adapter_pcb_box.scad`:

- `board_w`, `board_d`, `board_h` — adapter PCB pocket size.
- `render_part` — export `box`, `lid`, or `wing`.
- `mount_span_w` — 96 mm distance between wing pin holes / e-ink holder width.
- `wing_plug_*` — vertical plug on the wing and matching top sockets in the box body.
- `wing_socket_cap_t` — 2 mm top layer on wing bosses with through holes for wing insertion.
- `wing_socket_wall` — material thickness around each wing hole in the side bosses.
- `mount_wing_*` — side wing size and screen-holder pin hole alignment.

Re-export STL after changing the `.scad` file:

```bash
openscad -D 'render_part="box"' -o adapter_pcb_box.stl adapter_pcb_box.scad
openscad -D 'render_part="wing"' -o adapter_pcb_box_wing.stl adapter_pcb_box.scad
openscad -D 'render_part="lid"' -o adapter_pcb_box_lid.stl adapter_pcb_box.scad
```
