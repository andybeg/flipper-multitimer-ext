// Open-top box for a 79 x 26 x 5 mm adapter PCB with room for a 2 mm lid above it.
// Side mount wings are separate parts that drop into vertical sockets on the box top.
// Units: millimeters.
//
// Coordinate system:
// X - board width, Y - board depth, Z - vertical.
// Origin is the outside corner of the bottom face (left/front/bottom).

$fn = 48;

/* [Part] */

render_part = "box"; // [box, lid, wing]

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

/* [Mount Span] */

mount_span_w = 96.0; // [80:0.1:130]

/* [Screen Holder Mount Wings] */

wing_width = 4.0; // [2:0.5:10]
wing_height_z = 100.0; // [40:1:150]
pin_hole_clearance = 0.4; // [0.1:0.1:1]
wing_plug_depth = 4.0; // [2:0.5:10]
wing_plug_clearance = 0.25; // [0.05:0.05:0.6]
wing_socket_y_margin = 0.5; // [0:0.1:2]
wing_socket_cap_t = 2.0; // [1:0.5:5]
wing_socket_wall = 2.0; // [1:0.5:5]

/* [Derived Values] */

inner_w = board_w + board_clearance;
inner_d = board_d + board_clearance;
inner_h = board_h + lid_t - retention_h;
// Center shell width: pin-to-pin span minus both wing bosses (see mount_wing()).
outer_w_center = mount_span_w - 1.5 * (wing_width + 2 * wing_socket_wall);
wing_plug_w = wing_width;
wing_boss_w = wing_plug_w + 2 * wing_socket_wall;
total_outer_w = outer_w_center + 2 * wing_boss_w;
outer_d = 2 * (inner_d + 2 * wall);
outer_h = wall + inner_h;
wing_mount_z = outer_h + wing_socket_cap_t;
inner_origin_x_local = (outer_w_center - inner_w) / 2;
inner_origin_x = wing_boss_w + inner_origin_x_local;
inner_origin_y = wall;
holder_body_h = 78.5 * 0.50 + 4.0 + 0.6;
holder_pin_d = 2.4 + 5.0 + 2.0;
holder_pin_y = holder_body_h - holder_pin_d / 2;
holder_pin_z = wing_height_z - holder_pin_d / 2 - 2;
pin_hole_d = holder_pin_d + pin_hole_clearance;
wing_length_y = pin_hole_d + 4;
wing_y0 = holder_pin_y - wing_length_y / 2;
wing_plug_y = wing_length_y - 2 * wing_socket_y_margin;
wing_plug_total_depth = wing_socket_cap_t + wing_plug_depth;
wing_socket_depth = wing_plug_total_depth + wing_plug_clearance;
wing_panel_h = wing_height_z - wing_mount_z;
wing_pin_z = holder_pin_z - wing_mount_z;

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
        translate([inner_origin_x_local, inner_origin_y, wall])
            cube([inner_w, inner_d, inner_h + 0.1]);
    }
}

module inner_retention_ledges() {
    z0 = wall;

    translate([inner_origin_x, inner_origin_y, z0])
        cube([retention_w, inner_d, retention_h]);
    translate([inner_origin_x + inner_w - retention_w, inner_origin_y, z0])
        cube([retention_w, inner_d, retention_h]);
}

module inner_center_bumps() {
    x0 = inner_origin_x + (inner_w - center_bump_size) / 2;
    z0 = wall;

    translate([x0, inner_origin_y, z0])
        cube([center_bump_size, center_bump_size, retention_h]);
}

module side_bosses() {
    cube([wing_boss_w, outer_d, outer_h]);
    translate([wing_boss_w + outer_w_center, 0, 0])
        cube([wing_boss_w, outer_d, outer_h]);
}

module side_wing_caps() {
    // 2 mm top layer on wing bosses; vertical holes are cut separately.
    translate([0, 0, outer_h])
        cube([wing_boss_w, outer_d, wing_socket_cap_t]);
    translate([wing_boss_w + outer_w_center, 0, outer_h])
        cube([wing_boss_w, outer_d, wing_socket_cap_t]);
}

module wing_socket_cut_top(is_right) {
    boss_x0 = is_right ? (wing_boss_w + outer_w_center) : 0;
    plug_x = boss_x0 + (wing_boss_w - wing_plug_w) / 2;
    y0 = wing_y0 + wing_socket_y_margin;

    translate([plug_x, y0, outer_h + wing_socket_cap_t - wing_socket_depth])
        cube([
            wing_plug_w + wing_plug_clearance,
            wing_plug_y + wing_plug_clearance,
            wing_socket_depth + 0.1
        ]);
}

module wing_body() {
    // Print standing on the bed: plug at the bottom, panel above, pin hole through the panel.
    // Print two copies; mirror one in the slicer for the opposite side.
    pin_y = holder_pin_y - wing_y0;

    difference() {
        union() {
            translate([(wing_width - wing_plug_w) / 2, wing_socket_y_margin, 0])
                cube([wing_plug_w, wing_plug_y, wing_plug_total_depth]);
            translate([0, 0, wing_plug_total_depth])
                cube([wing_width, wing_length_y, wing_panel_h]);
        }

        translate([wing_width / 2, pin_y, wing_plug_total_depth + wing_pin_z])
            rotate([0, 90, 0])
                cylinder(h = wing_width + 0.2, d = pin_hole_d, center = true);
    }
}

module mount_wing(is_right) {
    boss_x0 = is_right ? (wing_boss_w + outer_w_center) : 0;
    plug_x = boss_x0 + (wing_boss_w - wing_plug_w) / 2;

    translate([plug_x, wing_y0, wing_mount_z])
        if(is_right) {
            mirror([1, 0, 0])
                translate([-wing_width, 0, 0])
                    wing_body();
        } else {
            translate([-wing_width, 0, 0])
                wing_body();
        }
}

module main_box_body() {
    difference() {
        union() {
            translate([wing_boss_w, 0, 0])
                rounded_box_shell(outer_w_center, outer_d, outer_h, corner_r);
            side_bosses();
            side_wing_caps();
            inner_retention_ledges();
            inner_center_bumps();
        }
        wing_socket_cut_top(false);
        wing_socket_cut_top(true);
    }
}

module box() {
    main_box_body();
}

module wing() {
    wing_body();
}

module lid() {
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
} else if(render_part == "wing") {
    wing();
} else {
    lid();
}
