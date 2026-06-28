# Flipper E-Ink Enclosure Parts

Parametric OpenSCAD models for mounting a 4.2" e-paper module with Flipper Zero.

## Files

- `eink_screen_holder.scad` — angled screen holder with slide-in grooves and bottom mount tabs.
- `eink_screen_holder.stl` — exported holder STL.
- `adapter_pcb_box.scad` — open-top box and optional lid for the adapter PCB.

## Before Printing

Measure your exact hardware and tune values in the `.scad` files.

`eink_screen_holder.scad`:

- `screen_board_w`, `screen_board_h` — WeAct e-paper PCB size.
- `holder_ratio` — how much of the board height the holder supports.
- `slot_gap`, `slot_wall`, `slot_lip` — slide-in groove dimensions.
- `tab_*` — bottom tabs for attaching the holder to the adapter box.

`adapter_pcb_box.scad`:

- `board_w`, `board_d`, `board_h` — adapter PCB pocket size.
- `mount_wing_*` — side wings for mounting the box to the screen holder.

Export STL from OpenSCAD after tuning the dimensions.
