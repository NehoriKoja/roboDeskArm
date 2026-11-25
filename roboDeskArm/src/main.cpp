#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include "claw.h"
#include "NewServo.h"

Claw* claw = new Claw(1, 45);
NewServo* baseServo = new NewServo(2, 180, 0);
NewServo* armServo = new NewServo(3, 100, 0);

const char* ssid = "ESP32-C3";
const char* password = "12345678";

WebServer server(80); // Create web server on port 80

// HTML webpage with virtual joystick and claw buttons
const char* htmlPage = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1, maximum-scale=1, user-scalable=no">
  <title>ESP32 Joystick Controller</title>
  <style>
    * {
      box-sizing: border-box;
      touch-action: manipulation;
    }
    body {
      font-family: Arial, sans-serif;
      background-color: #121212;
      color: white;
      margin: 0;
      padding: 10px;
      height: 100vh;
      display: flex;
      align-items: center;
      justify-content: center;
      overflow: hidden;
    }
    .container {
      display: flex;
      width: 100%;
      max-width: 900px;
      height: 100%;
      justify-content: space-between;
      align-items: center;
    }
    
    /* Joystick Container */
    #joystick-container {
      width: 160px;
      height: 160px;
      background-color: #1e1e1e;
      border: 2px solid #444;
      border-radius: 50%;
      position: relative;
      touch-action: none;
      display: flex;
      align-items: center;
      justify-content: center;
    }
    #joystick-handle {
      width: 70px;
      height: 70px;
      background-color: #007bff;
      border-radius: 50%;
      position: absolute;
      top: 45px;
      left: 45px;
      box-shadow: 0 4px 8px rgba(0,0,0,0.3);
    }

    /* Action Buttons (Right Side) */
    .actions {
      display: flex;
      flex-direction: column;
      gap: 20px;
    }
    .btn {
      width: 90px;
      height: 90px;
      border-radius: 50%;
      border: 2px solid #444;
      color: white;
      display: flex;
      justify-content: center;
      align-items: center;
      cursor: pointer;
      text-decoration: none;
      user-select: none;
      -webkit-tap-highlight-color: transparent;
    }
    .btn:active {
      opacity: 0.8;
    }
    .btn svg {
      width: 45px;
      height: 45px;
      fill: currentColor;
    }
    .claw-open { background-color: #28a745; }
    .claw-close { background-color: #dc3545; }
  </style>
</head>
<body>

  <div class="container">
    <!-- Left Side: Virtual Joystick -->
    <div id="joystick-container">
      <div id="joystick-handle"></div>
    </div>

    <!-- Right Side: Claw Controls -->
    <div class="actions">
      <a href="#" onclick="sendAction('/claw-open'); return false;" class="btn claw-open">
        <svg viewBox="0 0 24 24"><path d="M6 5v2c0 3.87 3.13 7 7 7s7-3.13 7-7V5M3 13h4M17 13h4"/></svg>
      </a>
      <a href="#" onclick="sendAction('/claw-close'); return false;" class="btn claw-close">
        <svg viewBox="0 0 24 24"><path d="M6 5v2c0 3.87 3.13 7 7 7s7-3.13 7-7V5M8 13h8"/></svg>
      </a>
    </div>
  </div>

  <script>
    const container = document.getElementById('joystick-container');
    const handle = document.getElementById('joystick-handle');
    
    let maxDist = 45; // Max radius the joystick can move
    let centerX = 80;
    let centerY = 80;
    let isDragging = false;
    let lastSendTime = 0;

    function sendJoystickData(x, y) {
      // Throttle requests to every 50ms so we don't spam the ESP32
      let now = Date.now();
      if (now - lastSendTime > 50) {
        lastSendTime = now;
        fetch(`/move?x=${Math.round(x)}&y=${Math.round(y)}`).catch(err => console.log(err));
      }
    }

    function sendAction(endpoint) {
      fetch(endpoint).catch(err => console.log(err));
    }

    // Touch and Mouse Event Listeners
    function handleStart(e) {
      isDragging = true;
      handleMove(e);
    }

    function handleEnd() {
      isDragging = false;
      handle.style.transform = `translate(0px, 0px)`;
      sendJoystickData(0, 0); // Send stop command when released
    }

    function handleMove(e) {
      if (!isDragging) return;
      let clientX = e.touches ? e.touches[0].clientX : e.clientX;
      let clientY = e.touches ? e.touches[0].clientY : e.clientY;
      
      let rect = container.getBoundingClientRect();
      let x = clientX - rect.left - centerX;
      let y = clientY - rect.top - centerY;
      
      let dist = Math.sqrt(x * x + y * y);
      if (dist > maxDist) {
        x *= maxDist / dist;
        y *= maxDist / dist;
      }
      
      handle.style.transform = `translate(${x}px, ${y}px)`;
      
      // Normalize values between -100 and 100 (Invert Y so up is positive)
      let normX = (x / maxDist) * 100;
      let normY = -(y / maxDist) * 100;
      
      sendJoystickData(normX, normY);
    }

    container.addEventListener('mousedown', handleStart);
    window.addEventListener('mousemove', handleMove);
    window.addEventListener('mouseup', handleEnd);

    container.addEventListener('touchstart', handleStart);
    window.addEventListener('touchmove', handleMove);
    window.addEventListener('touchend', handleEnd);
  </script>

</body>
</html>
)rawliteral";

void handleRoot() {
  server.send(200, "text/html", htmlPage);
}

// Read Joystick X and Y coordinates (-100 to 100)
void handleMove() {
  if (server.hasArg("x") && server.hasArg("y")) {
    int xVal = server.arg("x").toInt();
    int yVal = server.arg("y").toInt();

    // Map joystick (-100 to 100) to servo angle limits
    // For baseServo (0 to 180, center is 90)
    int baseAngle = map(yVal, -100, 100, 0, 180);
    
    // For armServo (0 to 100, center is 50)
    int armAngle = map(xVal, -100, 100, 0, 100);
    
    Serial.print("Joystick -> X: ");
    Serial.print(xVal);
    Serial.print(" | Y: ");
    Serial.println(yVal);
  }
  server.send(200, "text/plain", "OK");
}

void handleOpen() {
  Serial.println("Action: OPEN CLAW");
  claw->open();
  server.send(200, "text/plain", "OK");
}

void handleClose() {
  Serial.println("Action: CLOSE CLAW");
  claw->close();
  server.send(200, "text/plain", "OK");
}

void setup() {
  Serial.begin(115200);
  delay(3000);

  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);

  // Define webpage routes
  server.on("/", handleRoot);
  server.on("/move", handleMove);
  server.on("/claw-open", handleOpen);
  server.on("/claw-close", handleClose);

  server.begin();
  Serial.println("Web server started! Open http://192.168.4.1 on your phone.");
}

void loop() {
  server.handleClient();
}