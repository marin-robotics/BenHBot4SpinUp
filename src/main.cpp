#include "main.h"
#include "display/lv_objx/lv_list.h"
#include "pros/adi.hpp"
#include "pros/llemu.hpp"
#include "pros/misc.h"
#include "pros/misc.hpp"
#include "pros/motors.hpp"
#include "pros/rtos.hpp"
#include "pros/vision.hpp"
#include "pros/vision.h"
#include <string>
#define DIGITAL_SENSOR_PORT_A 'A'
#define DIGITAL_SENSOR_PORT_B 'B'
#define DIGITAL_SENSOR_PORT_C 'C'


// Variables
bool launcher_toggle = false, snarfer_toggle = false;
int left_x, left_y, right_x, right_y, snarfer, firing_input, launcher;
int forward_table[] = {
  -127,-125,-123,-121,-119,-117,-115,-113,-112,-110,-108,-106,-104,-102,-101,-99,-97,-95,-94,-92,-90,-88,-87,-85,-84,-82,-80,-79,-77,-76,-74,-73,-71,-70,-68,-67,-65,-64,-62,-61,-60,-58,-57,-56,-54,-53,-52,-50,-49,-48,-47,-45,-44,-43,-42,-41,-40,-39,-37,-36,-35,-34,-33,-32,-31,-30,-29,-28,-27,-26,-26,-25,-24,-23,-22,-21,-20,-20,-19,-18,-17,-17,-16,-15,-15,-14,-13,-13,-12,-11,-11,-10,-10,-9,-9,-8,-8,-7,-7,-6,-6,-5,-5,-5,-4,-4,-3,-3,-3,-3,-2,-2,-2,-2,-1,-1,-1,-1,-1,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,2,2,2,2,3,3,3,3,4,4,5,5,5,6,6,7,7,8,8,9,9,10,10,11,11,12,13,13,14,15,15,16,17,17,18,19,20,20,21,22,23,24,25,26,26,27,28,29,30,31,32,33,34,35,36,37,39,40,41,42,43,44,45,47,48,49,50,52,53,54,56,57,58,60,61,62,64,65,67,68,70,71,73,74,76,77,79,80,82,84,85,87,88,90,92,94,95,97,99,101,102,104,106,108,110,112,113,115,117,119,121,123,125,127
};
int turn_table[] = {
63,-61,-59,-57,-55,-54,-52,-50,-49,-47,-45,-44,-42,-41,-39,-38,-37,-35,-34,-33,-32,-31,-29,-28,-27,-26,-25,-24,-23,-22,-21,-21,-20,-19,-18,-17,-17,-16,-15,-15,-14,-13,-13,-12,-11,-11,-10,-10,-9,-9,-9,-8,-8,-7,-7,-7,-6,-6,-5,-5,-5,-5,-4,-4,-4,-4,-3,-3,-3,-3,-3,-2,-2,-2,-2,-2,-2,-2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,3,3,3,3,3,4,4,4,4,5,5,5,5,6,6,7,7,7,8,8,9,9,9,10,10,11,11,12,13,13,14,15,15,16,17,17,18,19,20,21,21,22,23,24,25,26,27,28,29,31,32,33,34,35,37,38,39,41,42,44,45,47,49,50,52,54,55,57,59,61,63
};
int launcher_cycle[] = {77,82,87,92,97,112, 117, 122, 127};
int launcher_power = 5; // default power level

// auton

float Wheel_Diameter = 4;
float Wheel_Circumference = Wheel_Diameter * 3.1416;
float Turning_Diameter = 14.6;
float Turning_Circumference = Turning_Diameter * 3.1416;
float Turning_Distance = 0;
float Wheel_Revolutions = 0;
float Turn_Wheel_Rotation = 0;
float Forward_Wheel_Rotation = 0;
float Turn_Tuning_Factor = 1;
float Move_Tuning_Factor = 1.05;
bool wait = false; 



// Defining ports
pros::Motor left_front_motor(1,pros::E_MOTOR_GEAR_GREEN, false, pros::E_MOTOR_ENCODER_DEGREES);
pros::Motor right_front_motor(10,pros::E_MOTOR_GEAR_GREEN, false, pros::E_MOTOR_ENCODER_DEGREES);
pros::Motor left_back_motor(11,pros::E_MOTOR_GEAR_GREEN, false, pros::E_MOTOR_ENCODER_DEGREES);
pros::Motor right_back_motor(20,pros::E_MOTOR_GEAR_GREEN, false, pros::E_MOTOR_ENCODER_DEGREES);
pros::Motor snarfer_motor(2);
pros::Motor secondary_snarfer_motor(4);
pros::Motor launcher_motor(13,pros::E_MOTOR_GEAR_BLUE, true, pros::E_MOTOR_ENCODER_DEGREES);
pros::Motor roller_motor(3);

// pneumatics
pros::ADIDigitalOut firing_pneumatic(DIGITAL_SENSOR_PORT_A);
pros::ADIDigitalOut endgame(DIGITAL_SENSOR_PORT_B);
pros::ADIDigitalOut net_launch(DIGITAL_SENSOR_PORT_C);

// Set motor groups
pros::Motor_Group left_motors ({left_front_motor, left_back_motor});
pros::Motor_Group right_motors ({right_front_motor, right_back_motor});

// Controller
pros::Controller controller(pros::E_CONTROLLER_MASTER);

void on_center_button() {
	static bool pressed = false;
	pressed = !pressed;
	if (pressed) {
		pros::lcd::set_text(2, "I was pressed!");
	} else {
		pros::lcd::clear_line(2);
	}
}

/**
 * Runs initialization code. This occurs as soon as the program is started.
 *
 * All other competition modes are blocked by initialize; it is recommended
 * to keep execution time for this mode under a few seconds.
 */
void initialize() {
	pros::lcd::initialize();
	pros::lcd::register_btn1_cb(on_center_button);  
  left_front_motor.set_brake_mode(pros::E_MOTOR_BRAKE_HOLD);
  left_back_motor.set_brake_mode(pros::E_MOTOR_BRAKE_HOLD);
  right_front_motor.set_brake_mode(pros::E_MOTOR_BRAKE_HOLD);
  right_back_motor.set_brake_mode(pros::E_MOTOR_BRAKE_HOLD);
  


}

/**
 * Runs while the robot is in the disabled state of Field Management System or
 * the VEX Competition Switch, following either autonomous or opcontrol. When
 * the robot is enabled, this task will exit.
 */
void disabled() {
	
}

/**
 * Runs after initialize(), and before autonomous when connected to the Field
 * Management System or the VEX Competition Switch. This is intended for
 * competition-specific initialization routines, such as an autonomous selector
 * on the LCD.
 *
 * This task will exit when the robot is enabled and autonomous or opcontrol
 * starts.
 */
void competition_initialize() {}

/**
 * Runs the user autonomous code. This function will be started in its own task
 * with the default priority and stack size whenever the robot is enabled via
 * the Field Management System or the VEX Competition Switch in the autonomous
 * mode. Alternatively, this function may be called in initialize or opcontrol
 * for non-competition testing purposes.
 *
 * If the robot is disabled or communications is lost, the autonomous task
 * will be stopped. Re-enabling the robot will restart the task, not re-start it
 * from where it left off.
 */

bool check_left_thresholds(float left_goal, float threshold) {
  if ((left_front_motor.get_position() > left_goal+threshold) || left_front_motor.get_position() < left_goal-threshold) {
    return true;
  }
  if ((left_back_motor.get_position() > left_goal+threshold) || left_back_motor.get_position() < left_goal-threshold) {
    return true;
  }
  return false;
}
bool check_right_thresholds(float right_goal, float threshold) {
  if ((right_front_motor.get_position() > right_goal+threshold) || right_front_motor.get_position() < right_goal-threshold) {
    return true;
  }
  if ((right_back_motor.get_position() > right_goal+threshold) || right_back_motor.get_position() < right_goal-threshold) {
    return true;
  }
  return false;
}
void turn(float angle, float velocity){
  wait = true; // Stop turn from running multiple times simultaneously
  
  Turning_Distance = angle/360 * Turning_Circumference;
  Wheel_Revolutions = Turning_Distance/Wheel_Circumference;
  Turn_Wheel_Rotation = Turn_Tuning_Factor*Wheel_Revolutions*360; // Degrees that the wheel should turn

  left_motors.tare_position();
  right_motors.tare_position();

  left_motors.move_relative(Turn_Wheel_Rotation, velocity);
  right_motors.move_relative(Turn_Wheel_Rotation, -velocity);
  while (check_left_thresholds(Turn_Wheel_Rotation, 10)) {
    pros::delay(2);
  }
  left_motors.tare_position();
  right_motors.tare_position();
  
  wait = false; // Allow the next movement request to run
}
void left_pivot_turn(float angle, float velocity){
  wait = true; // Stop turn from running multiple times simultaneously
  
  Turning_Distance = angle/180 * Turning_Circumference;
  Wheel_Revolutions = Turning_Distance/Wheel_Circumference;
  Turn_Wheel_Rotation = Turn_Tuning_Factor * Wheel_Revolutions*360; // Degrees that the wheel should turn

  right_motors.tare_position();

  right_motors.move_relative(Turn_Wheel_Rotation, -velocity);
  while (check_right_thresholds(Turn_Wheel_Rotation, 10)) {
    pros::delay(2);
  }
  right_motors.tare_position();
  
  wait = false; // Allow the next movement request to run
}
void right_pivot_turn(float angle, float velocity){
  wait = true; // Stop turn from running multiple times simultaneously
  
  Turning_Distance = angle/180 * Turning_Circumference;
  Wheel_Revolutions = Turning_Distance/Wheel_Circumference;
  Turn_Wheel_Rotation = Turn_Tuning_Factor * Wheel_Revolutions*360; // Degrees that the wheel should turn

  left_motors.tare_position();

  left_motors.move_relative(Turn_Wheel_Rotation, velocity);
  while (check_left_thresholds(Turn_Wheel_Rotation, 10)) {
    pros::delay(2);
  }
  left_motors.tare_position();
  
  wait = false; // Allow the next movement request to run
}
void move(float inches, float velocity) { // Movement request to move the bot a set distance at a set speed.
  wait = true;
  
  Forward_Wheel_Rotation = Move_Tuning_Factor*(inches/Wheel_Circumference)*360;

  left_motors.tare_position();
  right_motors.tare_position();

  left_motors.move_relative(Forward_Wheel_Rotation,velocity);
  right_motors.move_relative(-Forward_Wheel_Rotation,velocity);
  while (check_left_thresholds(Forward_Wheel_Rotation, 270)) {
    // Continue running this loop as long as the motors are not within +-5 units of its goal
    pros::lcd::set_text(1,"Running Delay Loop");
    pros::delay(2);
  }
  left_motors.tare_position();
  right_motors.tare_position();

  wait = false;
}
void simple_fire(int delay) {
  wait = true;
  firing_pneumatic.set_value(true);
  pros::delay(600);
  firing_pneumatic.set_value(false);
  pros::delay(delay);
  wait = false;
}
void auto_roller(int delay, int move_velocity){
  wait = true;
  roller_motor = -127;
  left_motors.move(move_velocity);
  right_motors.move(-move_velocity);
  pros::delay(delay);
  roller_motor = 0;
  left_motors.move(0);
  right_motors.move(0);
  wait = false;
}
/*
void snarfer_loop() {
  while (true){
    pros::lcd::set_text(3, "Snarf Loop");
    if (snarfer_toggle == true){
      snarfer_motor = -127;
    } else {
      snarfer_motor = 0;
    }
  }
}

void launcher_loop() {
  while (true){
    if (launcher_toggle == true){
      launcher_motor = -127;
    } else {
      launcher_motor = 0;
    }
  }
}
*/
void auto_fire() {
  if ((firing_input == 1) && (launcher_toggle)) {
    firing_pneumatic.set_value(true);
    for (int i=0; i<60 ; i++) {
      if ((firing_input == 1) && (launcher_toggle)){
        pros::delay(10);
      } else {
        firing_pneumatic.set_value(false);
        break;
      }
    }
    firing_pneumatic.set_value(false);
    for (int i=0; i<70 ; i++) {
      if ((firing_input == 1) && (launcher_toggle)){
        pros::delay(10);
      } else {
        break;
      }
    }
  } else {
    firing_pneumatic.set_value(false);
  }
}

void auton1(){ //Roller and 5 disks on wide area
  launcher_motor = 100; 
  
  while (wait) {pros::delay(10);} auto_roller(400,60); pros::delay(400);
  
  while (wait) {pros::delay(10);} right_pivot_turn(-40,100); pros::delay(400);
  
  while (wait) {pros::delay(10);} turn(-50,100); pros::delay(500);

  while (wait) {pros::delay(10);} simple_fire(700); pros::delay(200);

  while (wait) {pros::delay(10);} simple_fire(700); pros::delay(200);

  launcher_motor = 0;
  snarfer_motor = -127;
  secondary_snarfer_motor = -127;

  while (wait) {pros::delay(10);} turn(-45,100); pros::delay(300);

  while (wait) {pros::delay(10);} move(40,90); pros::delay(500);

  secondary_snarfer_motor = 127;
  
  while (wait) {pros::delay(10);} move(20,80); pros::delay(500);

  while (wait) {pros::delay(10);} turn(90,50); pros::delay(1500);

  launcher_motor = 90; 
  snarfer_motor = 0;
  secondary_snarfer_motor = 0;

  pros::delay(1000);

  while (wait) {pros::delay(10);} simple_fire(700); pros::delay(200);

  while (wait) {pros::delay(10);} simple_fire(700); pros::delay(200);  

  while (wait) {pros::delay(10);} simple_fire(700); pros::delay(200);

  launcher_motor = 0;
}
void auton2(){ // Shoot two on small area

  launcher_motor = 80; pros::delay(1000);

  simple_fire(1000);
  
  simple_fire(1000); 
  
  pros::delay(1000);

  launcher_motor = 0; 
}

void auton3(){ // Face low goal, fire, then get roller
  launcher_motor = 100; pros::delay(1000);

  simple_fire(800);
  
  simple_fire(800);
  
  pros::delay(500);
  
  launcher_motor = 0; 

  //while (wait) {pros::delay(10);} turn(180,50); pros::delay(300);
  
  while (wait) {pros::delay(10);} move(-32,40); pros::delay(300);
  
  while (wait) {pros::delay(10);} turn(-95,50); pros::delay(500);

  while (wait) {pros::delay(10);} auto_roller(500,  80); 

  launcher_motor = 0; 

  while (wait) {pros::delay(10);} move(-3,40); pros::delay(300);

  while (wait) {pros::delay(10);} turn(-120,50); pros::delay(500);

}
void auton4(){ // Shoot two on small area
  while (wait) {pros::delay(10);} auto_roller(250, 50); pros::delay(400);
  while (wait) {pros::delay(10);} move(20,80); pros::delay(500);
  pros::delay(1000);

}

void autonomous() {
  pros::lcd::clear();
  pros::lcd::set_text(1, "Starting"); 
  auton4();




  //while (wait) {pros::delay(10);} turn(-360, 100); pros::delay(1000);

  

/**
 * Runs the operator control code. This function will be started in its own task
 * with the default priority and stack size whenever the robot is enabled via
 * the Field Management System or the VEX Competition Switch in the operator
 * control mode.
 *
 * If no competition control is connected, this function will run immediately
 * following initialize().
 *
 * If the robot is disabled or communications is lost, the
 * operator control task will be stopped. Re-enabling the robot will restart the
 * task, not resume it from where it left off. 
 */

}
// toggles for the pneumatics
void opcontrol() {
  firing_input = 0; // Stop auto firing pneumatic
  while (true) {
    pros::lcd::clear();
    // Get joystick values
    left_y = (controller.get_analog(pros::E_CONTROLLER_ANALOG_LEFT_Y));
    left_x = (controller.get_analog(pros::E_CONTROLLER_ANALOG_LEFT_X));
    right_y = (controller.get_analog(pros::E_CONTROLLER_ANALOG_RIGHT_Y));
    right_x = (controller.get_analog(pros::E_CONTROLLER_ANALOG_RIGHT_X));
    firing_input = (controller.get_digital(pros::E_CONTROLLER_DIGITAL_R1));
    pros::lcd::set_text(3, "Launching Motor Efficiency: " + std::to_string(launcher_motor.get_efficiency()));
    pros::lcd::set_text(4, "Launching Motor Temperature: " + std::to_string(launcher_motor.get_temperature()));
    pros::lcd::set_text(5, "Launching Motor Wattage: " + std::to_string(launcher_motor.get_power()));
    //controller.print(1, 1, "Power: " + std::to_string(launcher_power[launcher_cycle]), 0)

    if (controller.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_UP) && launcher_power < 8) {
      launcher_power += 1;
    }
    if (controller.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_DOWN) && launcher_power > 0) {
      launcher_power -= 1;
    }
    
    // Toggle launcher (R2)
    if (controller.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_R2) == 1) {
      launcher_toggle = !launcher_toggle;
    }
    if (launcher_toggle) {
      launcher_motor.move(launcher_cycle[launcher_power]);
    } else {
      launcher_motor.move(0);
    }

    // Toggle snarfer (L1)
    if (controller.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_L1) == 1) {
      snarfer_toggle = !snarfer_toggle;
    }
    if (snarfer_toggle) {
      snarfer_motor = -127;
      if (controller.get_digital(pros::E_CONTROLLER_DIGITAL_L2)){
        secondary_snarfer_motor = 127;
      } else {
        secondary_snarfer_motor = -127;
      }
    } else {
      snarfer_motor = 0;
      if (controller.get_digital(pros::E_CONTROLLER_DIGITAL_L2)){
        secondary_snarfer_motor = 127;
      } else {
        secondary_snarfer_motor = 0;
      }
    }

    // Roller motor (A)

    if (controller.get_digital(DIGITAL_A)) {
        roller_motor = -100;
    } else {
      roller_motor = 0;
    }


    // Fire pneumatic (R1)
    auto_fire();

    if (controller.get_digital(pros::E_CONTROLLER_DIGITAL_RIGHT) && (controller.get_digital(pros::E_CONTROLLER_DIGITAL_Y))){
      endgame.set_value(true);
    }

    // Fire net launcher
    if (controller.get_digital(DIGITAL_Y)){
      if (controller.get_digital(pros::E_CONTROLLER_DIGITAL_LEFT)){
        net_launch.set_value(true);
      }
    }

    /* Drive Control Loop (LEFT)
    if (controller.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_LEFT)) {
      one_stick = !one_stick;
    }
    */

    left_motors.move(forward_table[left_x+127] + forward_table[left_y+127]);
    right_motors.move(forward_table[left_x+127] - forward_table[left_y+127]);
    pros::lcd::set_text(1, "Left Motors Speed: " + std::to_string(forward_table[left_x+127] + forward_table[left_y+127]));
    pros::lcd::set_text(2, "Right Motors Speed: "+ std::to_string(forward_table[left_x+127] - forward_table[left_y+127]));

    pros::delay(20);
  }
}