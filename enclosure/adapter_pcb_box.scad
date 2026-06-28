// Open-top box for a 79 x 26 x 5 mm adapter PCB with room for a 2 mm lid above it.
// Units: millimeters.
//
// Coordinate system:
// X - board width, Y - board depth, Z - vertical.
// Origin is the outside corner of the bottom face.

$fn = 48;

/* [Part] */

render_part = "box"; // [box, lid]

/* [Board Pocket] */

board_w = 79.0; // [60:0.1:100]
board_d = 26.0; // [15:0.1:50]
board_h = 5.0; // [2:0.1:12]
board_clearance = 0.4; // [0.1:0.1:1.5]

/* [Lid Pocket] */

lid_t = 2.0; // [1:0.1:5]
lid_clearance = 0.3; // [0.1:0.1:1]

/* [Box Shell] */

wall = 2.0; // [1.2:0.1:4]
corner_r = 2.0; // [0:0.5:6]

/* [Inner Retention Ledges] */

retention_h = 1.0; // [0.5:0.1:3]
retention_w = 3.0; // [1:0.5:8]
center_bump_size = 7.0; // [3:0.5:15]

/* [Screen Holder Mount Wings] */

wing_width = 4.0; // [2:0.5:10]
wing_height_z = 100.0; // [40:1:150]
pin_hole_clearance = 0.4; // [0.1:0.1:1]

/* [Derived Values] */

inner_w = board_w + board_clearance;
inner_d = board_d + board_clearance;
// Height stack (Z): wall(2) + retention_h(1) + board_h(5) + lid_t(2) - overlap(1) = 8.
// The 1 mm ledges sit inside the board pocket, so they do not add outer height.
inner_h = board_h + lid_t - retention_h;
// Match eink_screen_holder body width along X, without side pin length.
screen_holder_body_w = 101.0 + 2 * (2.4 + 0.6);
outer_w = screen_holder_body_w;
outer_d = 2 * (inner_d + 2 * wall);
outer_h = wall + inner_h;
// Inner pocket centered on X, anchored to the front edge on Y.
inner_origin_x = (outer_w - inner_w) / 2;
inner_origin_y = wall;
// Screen holder side pin geometry (match eink_screen_holder.scad).
holder_body_h = 78.5 * 0.50 + 4.0 + 0.6;
holder_pin_d = 2.4 + 5.0 + 2.0;
holder_pin_y = holder_body_h - holder_pin_d / 2;
holder_pin_z = wing_height_z - holder_pin_d / 2 - 2;
pin_hole_d = holder_pin_d + pin_hole_clearance;
wing_length_y = pin_hole_d + 4;
wing_y0 = holder_pin_y - wing_length_y / 2;

module rounded_box_shell(w, d, h, r) {
    difference() {
        if(r <= 0) {
            cube([w, d, h]);
        } else {
            hull() {
                for(x = [r, w - r])
                    for(y = [r, d - r])
                        translate([x, y, 0])
                            cylinder(h = h, r = r);
            }
        }
        translate([inner_origin_x, inner_origin_y, wall])
            cube([inner_w, inner_d, inner_h + 0.1]);
    }
}

module inner_retention_ledges() {
    // Bottom ledges on the two opposite short inner walls, full side length (Y).
    z0 = wall;

    translate([inner_origin_x, inner_origin_y, z0])
        cube([retention_w, inner_d, retention_h]);
    translate([inner_origin_x + inner_w - retention_w, inner_origin_y, z0])
        cube([retention_w, inner_d, retention_h]);
}

module inner_center_bumps() {
    // Single centered 7 x 7 mm bottom bump on the front inner wall (y near 0).
    x0 = inner_origin_x + (inner_w - center_bump_size) / 2;
    z0 = wall;

    translate([x0, inner_origin_y, z0])
        cube([center_bump_size, center_bump_size, retention_h]);
}

module mount_wing(is_right) {
    x0 = is_right ? outer_w : -wing_width;
    x_center = is_right ? outer_w + wing_width / 2 : -wing_width / 2;

    difference() {
        translate([x0, wing_y0, 0])
            cube([wing_width, wing_length_y, wing_height_z]);

        translate([x_center, holder_pin_y, holder_pin_z])
            rotate([0, 90, 0])
                cylinder(h = wing_width + 0.2, d = pin_hole_d, center = true);
    }
}

module side_mount_wings() {
    mount_wing(false);
    mount_wing(true);
}

module main_box_body() {
    union() {
        rounded_box_shell(outer_w, outer_d, outer_h, corner_r);
        inner_retention_ledges();
        inner_center_bumps();
    }
}

module box() {
    union() {
        main_box_body();
        side_mount_wings();
    }
}

module lid() {
    // Flat insert that fits the open top of the cavity above the board.
    rounded_plate(inner_w - 2 * lid_clearance, inner_d - 2 * lid_clearance, lid_t, max(corner_r - wall, 0));
}

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

if(render_part == "box") {
    box();
} else {
    lid();
}
