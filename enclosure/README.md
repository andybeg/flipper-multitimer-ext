# Flipper E-Ink Mount Enclosure

Parametric OpenSCAD enclosure for mounting a 4.2" e-paper module with Flipper Zero.

## Files

- `flipper_eink_mount.scad` - source model.

## Before Printing

Measure your exact hardware and tune these values in the `.scad` file:

- `screen_board_w`, `screen_board_h` - physical size of the WeAct e-paper PCB.
- `gpio_center_z` - height of Flipper GPIO pin center from the table when Flipper lies on its back cover.
- `header_center_x`, `header_face_y` - adapter header position.
- `screen_angle_deg` - display angle.
- `screen_holder_ratio` - how much of the screen board height is supported by the lower holder.
- `screen_slot_gap`, `screen_slot_lip`, `screen_slot_wall` - side groove dimensions for sliding the e-paper PCB into place.
- `screen_screw_inset_x`, `screen_screw_inset_y` - mounting hole positions if you enable `use_screen_screw_holes`.

## Intended Layout

- Bottom rectangular box holds the adapter/interconnect PCB.
- Front face has 1x8 holes for `VCC GND SDA SCL CS D/C RES BUSY`.
- Pin holes are placed at `gpio_center_z` so they align with Flipper Zero GPIO height.
- E-paper module sits in angled side grooves above the box. The holder supports the lower half of the board and leaves the upper part open.

Export STL from OpenSCAD after tuning the dimensions.
