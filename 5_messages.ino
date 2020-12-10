
String getStatus (int plant) 
{
  // string variables
  String mode = "";
  String status = "";

  if (pumpOn[plant] == true) {
    // pump is on
    return "Watering ...";
    
  } else if (systemOn) {
    // determine mode
    switch (modes[plant]) {
      case 0:
        // not set up yet
        return "Not set up yet!";
        break;
      case 1:
        mode = "Manual";
        break;
      case 2:
        mode = "Auto";
        break;
      case 3:
        mode = "Timer";
        break;
    }

    // Determine time of last water
    if (lastWater[plant] == ontime) {
      status = "not watered.";
    } else {
      // calculate difference
      long diff = now() - lastWater[plant];

      // onvert between minutes, hours, days and too much
      if (diff < 60) {
        status = "just now.";

      } else if (diff < 2 * 60) {
        status = "1 min ago.";
        
      } else if (diff < 60 * 60) {
        status = String(diff / 60) + " mins ago.";
        
      } else if (diff < 60 * 60 * 2) {
        status = "1 hour ago";

      } else if (diff < 60 * 60 * 24) {
        status = String(diff / (60 * 60)) + " hours ago.";

      } else if (diff < 60 * 60 * 24 * 2) {
        status = "1 day ago";

      } else if (diff < 60 * 60 * 24 * 31) {
        status = String(diff / (60 * 60 * 24)) + " days ago.";

      } else {
        status = "+1 month ago.";
      }
    }

    // return concatenated results
    return mode + ",  " + status;

  } else {
    // system is off
    return "Off";
  } 
}
