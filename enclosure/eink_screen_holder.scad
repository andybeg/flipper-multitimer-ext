// Standalone 4.2" e-paper screen holder for MultiTimer Ext.
// Units: millimeters.
//
// This is only the display holder part. The lower adapter box will be modeled
// separately and can mount to this part through the bottom tabs.

$fn = 48;

/* [E-Ink Board] */

screen_board_w = 101.0; // [80:0.1:130]
screen_board_h = 78.5; // [60:0.1:100]
screen_board_t = 2.0; // [0.8:0.1:4]
screen_clearance = 0.6; // [0.1:0.1:2]

/* [Holder Shape] */

holder_ratio = 0.50; // [0.25:0.05:0.90]
back_plate_t = 2.4; // [1.2:0.1:5]
bottom_stop_h = 4.0; // [1:0.5:12]
corner_r = 2.5; // [0:0.5:8]

/* [Slide-In Grooves] */

slot_gap = 5.0; // [2:0.1:10]
slot_wall = 2.4; // [1.2:0.1:6]
slot_lip = 2.0; // [0.8:0.1:5]
slot_lip_t = 2.0; // [0.8:0.1:8]

/* [Mounting Tabs] */

tab_enabled = true;
tab_w = 18; // [8:1:35]
tab_d = 10; // [5:1:25]
tab_t = 3; // [1.5:0.1:6]
tab_hole_d = 3.2; // [2:0.1:5]
tab_offset_x = 36; // [15:1:55]

/* [Optional Screen Screws] */

use_screen_screw_holes = false;
screen_screw_d = 2.2; // [1.2:0.1:4]
screen_screw_inset_x = 5.0; // [2:0.5:20]
screen_screw_inset_y = 5.0; // [2:0.5:20]

/* [Side Pins] */

side_pin_len = 3.0; // [1:0.5:10]

/* [Derived Values] */

part_z_h = back_plate_t + slot_gap + slot_lip_t;
side_pin_d = part_z_h;
holder_w = screen_board_w + 2 * (slot_wall + screen_clearance);
holder_h = screen_board_h * holder_ratio + bottom_stop_h + screen_clearance;

module rounded_plate(w, h, t, r) {
    if(r <= 0) {
        cube([w, h, t]);
    } else {
        hull() {
            for(x = [r, w - r])
                for(y = [r, h - r])
                    translate([x, y, 0])
                        cylinder(h = t, r = r);
        }
    }
}

module screen_holder_body() {
    difference() {
        union() {
            // Half-height back plate. The board's upper half is unsupported.
            rounded_plate(holder_w, holder_h, back_plate_t, corner_r);

            // Bottom stop: the PCB rests on this after sliding into the grooves.
            translate([0, 0, back_plate_t])
                cube([holder_w, bottom_stop_h, slot_gap + slot_lip_t]);

            // Left and right outer groove walls.
            translate([0, 0, back_plate_t])
                cube([slot_wall, holder_h, slot_gap + slot_lip_t]);
            translate([holder_w - slot_wall, 0, back_plate_t])
                cube([slot_wall, holder_h, slot_gap + slot_lip_t]);

            // C-channel lips. These capture the front edge of the PCB.
            translate([slot_wall, bottom_stop_h, back_plate_t + slot_gap])
                cube([slot_lip, holder_h - bottom_stop_h, slot_lip_t]);
            translate([holder_w - slot_wall - slot_lip, bottom_stop_h, back_plate_t + slot_gap])
                cube([slot_lip, holder_h - bottom_stop_h, slot_lip_t]);
        }

        if(use_screen_screw_holes) {
            for(x = [
                    slot_wall + screen_clearance + screen_screw_inset_x,
                    slot_wall + screen_clearance + screen_board_w - screen_screw_inset_x
                ])
                for(y = [
                        bottom_stop_h + screen_screw_inset_y,
                        bottom_stop_h + screen_board_h * holder_ratio - screen_screw_inset_y
                    ])
                    translate([x, y, -0.2])
                        cylinder(h = back_plate_t + slot_gap + slot_lip_t + 0.6, d = screen_screw_d);
        }
    }
}

module mounting_tabs() {
    if(tab_enabled) {
        // Rear rail ties tabs into the holder body and gives the next enclosure
        // part a flat mating surface.
        translate([0, -tab_d, 0])
            cube([holder_w, tab_d, tab_t]);

        for(x = [holder_w / 2 - tab_offset_x, holder_w / 2 + tab_offset_x]) {
            difference() {
                translate([x - tab_w / 2, -tab_d, 0])
                    cube([tab_w, tab_d, tab_t]);
                translate([x, -tab_d / 2, -0.2])
                    cylinder(h = tab_t + 0.4, d = tab_hole_d);
            }
        }
    }
}

module side_edge_pins() {
    // Pins sit on the outer side faces only; aligned to the top edge of the holder.
    pin_y = holder_h - side_pin_d / 2;

    translate([0, pin_y, part_z_h / 2])
        rotate([0, -90, 0])
            cylinder(h = side_pin_len, d = side_pin_d);

    translate([holder_w, pin_y, part_z_h / 2])
        rotate([0, 90, 0])
            cylinder(h = side_pin_len, d = side_pin_d);
}

module part() {
    screen_holder_body();
    mounting_tabs();
    side_edge_pins();
}

// Clip negative tabs; keep side pins that extend beyond the holder width.
intersection() {
    part();
    translate([-side_pin_len, 0, 0])
        cube([holder_w + 2 * side_pin_len, holder_h, part_z_h + tab_t + 1]);
}
