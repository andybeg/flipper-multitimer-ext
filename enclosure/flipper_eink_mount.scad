// Parametric enclosure for mounting a 4.2" e-paper module with Flipper Zero.
// Units: millimeters.
//
// Coordinate system:
// X - left/right, Y - front/back, Z - vertical.
// The bottom box sits on the table. The e-paper tray rises from its back edge.
//
// Measure and tune before printing:
// - screen_board_w / screen_board_h for your exact WeAct board
// - gpio_center_z so the pin holes match Flipper Zero lying on its back
// - header_x/y/z parameters for your adapter PCB/header placement

$fn = 48;

/* [Base Box] */

base_w = 112; // [80:1:140]
base_d = 36; // [20:1:70]
base_h = 24; // [12:1:45]
wall = 2.2; // [1.2:0.1:4]
corner_r = 3; // [0:0.5:8]

/* [E-Ink Board] */

// Approximate 4.2" module dimensions. WeAct board may differ; measure yours.
screen_board_w = 101.0; // [80:0.1:130]
screen_board_h = 78.5; // [60:0.1:100]
screen_board_t = 2.0; // [0.8:0.1:4]
screen_clearance = 0.6; // [0.1:0.1:2]

/* [Screen Holder] */

// Tray is angled upward from the back of the base.
screen_angle_deg = 65; // [35:1:80]
tray_thickness = 2.4; // [1.2:0.1:5]
tray_rim_h = 3.0; // [1:0.1:8]
tray_rim_w = 2.4; // [1:0.1:6]
tray_back_offset = 4; // [0:0.5:15]
tray_hinge_offset_z = 1.5; // [0:0.5:8]

// The screen holder supports only the lower part of the e-paper board.
// The board slides into side grooves instead of being screwed to a full plate.
screen_holder_ratio = 0.50; // [0.25:0.05:0.90]
screen_slot_extra_clearance = 0.7; // [0.1:0.1:2]
screen_slot_lip = 2.0; // [0.8:0.1:5]
screen_slot_wall = 2.4; // [1.2:0.1:6]
screen_bottom_stop_h = 4.0; // [1:0.5:10]

/* [GPIO Header] */

// Header holes for 1x8 pin row: VCC GND SDA SCL CS D/C RES BUSY.
// These holes are on the front face of the bottom box.
header_pin_count = 8; // [1:1:12]
header_pitch = 2.54; // [2:0.01:3]
header_hole_d = 1.25; // [0.8:0.05:2]
header_hole_extra_depth = 3.0; // [0:0.5:8]
gpio_center_z = 17.0; // [8:0.1:30]
header_center_x = 0; // [-40:0.5:40]
header_face_offset_y = 0.05; // [0:0.05:2]

/* [Adapter Board Opening] */

adapter_board_w = 72; // [30:1:105]
adapter_board_d = 24; // [15:1:34]
adapter_board_clearance_z = 10; // [4:1:20]

/* [Optional Screen Screws] */

// Screw holes are off by default: the screen is held by slide-in grooves.
use_screen_screw_holes = false;
screen_screw_d = 2.2; // [1.2:0.1:4]
screen_screw_inset_x = 5.0; // [2:0.5:20]
screen_screw_inset_y = 5.0; // [2:0.5:20]

/* [Derived Values - usually do not edit] */

tray_back_y = base_d / 2 - tray_back_offset;
tray_hinge_z = base_h - tray_hinge_offset_z;
screen_slot_gap = screen_board_t + screen_slot_extra_clearance;
header_hole_depth = wall + header_hole_extra_depth;
header_face_y = -base_d / 2 - header_face_offset_y;

// ---- Helpers ----
module rounded_box(size, r) {
    hull() {
        for(x = [-size[0] / 2 + r, size[0] / 2 - r])
            for(y = [-size[1] / 2 + r, size[1] / 2 - r])
                translate([x, y, 0])
                    cylinder(h = size[2], r = r);
    }
}

module pin_header_holes() {
    row_w = (header_pin_count - 1) * header_pitch;
    for(i = [0 : header_pin_count - 1]) {
        x = header_center_x - row_w / 2 + i * header_pitch;
        translate([x, header_face_y, gpio_center_z])
            rotate([90, 0, 0])
                cylinder(h = header_hole_depth, d = header_hole_d);
    }
}

module base_box() {
    difference() {
        rounded_box([base_w, base_d, base_h], corner_r);

        // Hollow interior for adapter PCB and wiring.
        translate([0, 0, wall])
            rounded_box(
                [base_w - 2 * wall, base_d - 2 * wall, base_h],
                max(0.5, corner_r - wall)
            );

        // Top service opening.
        translate([0, 0, base_h - wall - 0.1])
            cube([adapter_board_w, adapter_board_d, wall + 0.4], center = true);

        pin_header_holes();

        // Cable slot from base box to the tilted tray.
        translate([0, base_d / 2 - wall / 2, base_h - 8])
            cube([26, wall + 1, 10], center = true);
    }
}

module screen_tray_flat() {
    tray_w = screen_board_w + 2 * (screen_slot_wall + screen_clearance);
    holder_h = screen_board_h * screen_holder_ratio;
    tray_h = holder_h + screen_bottom_stop_h + screen_clearance;

    difference() {
        union() {
            // Half-height back plate. The upper half of the screen remains open.
            translate([-tray_w / 2, 0, 0])
                cube([tray_w, tray_h, tray_thickness]);

            // Bottom stop: the board rests on this edge after sliding into grooves.
            translate([-tray_w / 2, 0, tray_thickness])
                cube([tray_w, screen_bottom_stop_h, tray_rim_h]);

            // Side groove walls.
            translate([-tray_w / 2, 0, tray_thickness])
                cube([screen_slot_wall, tray_h, screen_slot_gap + screen_slot_lip]);
            translate([tray_w / 2 - screen_slot_wall, 0, tray_thickness])
                cube([screen_slot_wall, tray_h, screen_slot_gap + screen_slot_lip]);

            // Top lips of the side grooves. These make a C-channel for the PCB edge.
            translate([
                -tray_w / 2 + screen_slot_wall,
                screen_bottom_stop_h,
                tray_thickness + screen_slot_gap
            ])
                cube([screen_slot_lip, tray_h - screen_bottom_stop_h, screen_slot_lip]);
            translate([
                tray_w / 2 - screen_slot_wall - screen_slot_lip,
                screen_bottom_stop_h,
                tray_thickness + screen_slot_gap
            ])
                cube([screen_slot_lip, tray_h - screen_bottom_stop_h, screen_slot_lip]);
        }

        // Optional screw holes through the tray.
        if(use_screen_screw_holes) {
            for(x = [
                    -screen_board_w / 2 + screen_screw_inset_x,
                    screen_board_w / 2 - screen_screw_inset_x
                ])
                for(y = [
                        tray_rim_w + screen_clearance + screen_screw_inset_y,
                        tray_rim_w + screen_clearance + screen_board_h - screen_screw_inset_y
                    ])
                    translate([x, y, -0.2])
                        cylinder(h = tray_thickness + tray_rim_h + 0.6, d = screen_screw_d);
        }
    }
}

module tilted_screen_tray() {
    translate([0, tray_back_y, tray_hinge_z])
        rotate([screen_angle_deg, 0, 0])
            screen_tray_flat();
}

module hinge_bridge() {
    // Mechanical bridge that makes the lower box and tilted tray one printable body.
    translate([0, base_d / 2 - 4, base_h - 3])
        cube([base_w - 8, 8, 8], center = true);
}

module center_spine() {
    // A broad internal spine that intentionally intersects both the base and tray.
    // This keeps the exported STL as a mechanically connected part.
    hull() {
        translate([0, base_d / 2 - 8, base_h - 5])
            cube([base_w - 18, 10, 8], center = true);
        translate([0, tray_back_y + 28, base_h + 30])
            cube([base_w - 22, 8, 8], center = true);
    }
}

module side_gusset(x) {
    // Diagonal support between bottom box and tilted tray.
    support_t = 4;
    hull() {
        translate([x, base_d / 2 - 7, base_h - 3])
            cube([support_t, 10, 6], center = true);
        translate([x, tray_back_y + 20, base_h + 24])
            cube([support_t, 10, 6], center = true);
    }
}

module assembly() {
    base_box();
    hinge_bridge();
    center_spine();
    tilted_screen_tray();
    side_gusset(-base_w / 2 + 10);
    side_gusset(base_w / 2 - 10);
}

assembly();
