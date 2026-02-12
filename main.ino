#include <WiFi.h>
#include <WebServer.h>
// enables .local domain
#include <ESPmDNS.h>


const char* ssid     = "my-wifi-name";
const char* password = "my-super-secret-wifi-password";
const char* hostname = "walnut"; 

WebServer server(80);

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
  <title>The Walnut Server</title>
  <style>
    :root {
      --bg-gradient: radial-gradient(circle at 50% -20%, #2b1a0e, #0f0f0f 80%);
      --glass-bg: rgba(255, 255, 255, 0.03);
      --glass-border: rgba(255, 255, 255, 0.08);
      --accent: #d4a373;
      --accent-glow: rgba(212, 163, 115, 0.4);
      --text-main: #ffffff;
      --text-muted: #888888;
      --status-green: #4caf50;
    }
    * { box-sizing: border-box; margin: 0; padding: 0; }
    
    body {
      font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif;
      background: var(--bg-gradient);
      color: var(--text-main);
      min-height: 100vh; 
      display: flex;
      align-items: center;
      justify-content: center;
      overflow: hidden;
      transition: background 0.8s ease; /* Smooth transition to space */
    }

    /* --- SPACE MODE STYLES --- */
    body.space-active {
      background: radial-gradient(circle at 50% 50%, #1a1a2e, #000000);
    }
    /* Hide the brown orbs when in space */
    body.space-active .orb { opacity: 0; }
    /* Show the stars when in space */
    body.space-active .star-container { opacity: 1; }

    .orb { position: absolute; border-radius: 50%; filter: blur(80px); opacity: 0.4; z-index: -1; transition: opacity 0.8s ease; }
    .orb-1 { width: 300px; height: 300px; background: #6b4c35; top: -50px; left: -50px; animation: drift 10s infinite alternate; }
    .orb-2 { width: 250px; height: 250px; background: #3d2b1f; bottom: -50px; right: -50px; animation: drift 12s infinite alternate-reverse; }
    
    .card {
      position: relative;
      width: 90%; 
      max-width: 380px;
      margin: auto; 
      padding: 40px 30px;
      background: var(--glass-bg);
      backdrop-filter: blur(20px);
      -webkit-backdrop-filter: blur(20px);
      border: 1px solid var(--glass-border);
      border-radius: 30px;
      text-align: center;
      box-shadow: 0 25px 50px -12px rgba(0, 0, 0, 0.5);
      animation: float 6s ease-in-out infinite;
      z-index: 10;
    }

    .nut-img {
      width: 140px; height: auto; margin-bottom: 20px; display: inline-block;
      filter: drop-shadow(0 0 20px var(--accent-glow));
    }

    h1 {
      font-weight: 800; font-size: 28px; letter-spacing: -0.5px; margin-bottom: 8px;
      background: linear-gradient(to right, #fff, #bbb); -webkit-background-clip: text; -webkit-text-fill-color: transparent;
    }
    .badge {
      display: inline-block; font-size: 10px; font-weight: 700; text-transform: uppercase; letter-spacing: 1.5px;
      color: var(--accent); border: 1px solid rgba(212, 163, 115, 0.3); padding: 6px 12px; border-radius: 20px;
      margin-bottom: 24px; background: rgba(212, 163, 115, 0.05);
    }
    .info-group { text-align: left; margin-top: 30px; padding-top: 20px; border-top: 1px solid var(--glass-border); }
    .info-item { display: flex; justify-content: space-between; margin-bottom: 12px; font-size: 13px; }
    .label { color: var(--text-muted); }
    .value { font-weight: 600; font-family: 'Courier New', monospace; letter-spacing: -0.5px; display: flex; align-items: center; }
    .footer-note { font-size: 12px; color: var(--text-muted); margin-top: 25px; line-height: 1.5; font-style: italic; }

    .status-dot {
      width: 8px; height: 8px; background-color: var(--status-green); border-radius: 50%; margin-right: 8px;
      box-shadow: 0 0 0 0 rgba(76, 175, 80, 1); animation: pulse-green 2s infinite;
    }

    /* --- SPACE BUTTON --- */
    .space-btn {
      margin-top: 20px;
      padding: 10px 24px;
      background: rgba(255,255,255,0.05);
      border: 1px solid var(--glass-border);
      color: var(--text-muted);
      font-size: 12px;
      text-transform: uppercase;
      letter-spacing: 2px;
      border-radius: 50px;
      cursor: pointer;
      transition: all 0.4s ease;
    }
    /* When hovering the button, it glows blue/white */
    .space-btn:hover {
      background: rgba(255, 255, 255, 0.15);
      color: #fff;
      border-color: #fff;
      box-shadow: 0 0 15px rgba(255,255,255,0.4);
    }

    /* --- STARS ANIMATION --- */
    .star-container {
      position: fixed; top: 0; left: 0; width: 100%; height: 100%;
      pointer-events: none; z-index: -2; opacity: 0; transition: opacity 1s;
    }
    .star {
      position: absolute; background: white; border-radius: 50%; opacity: 0.8;
      animation: fly linear infinite;
    }
    @keyframes fly {
      from { transform: translateY(0) scale(0.5); opacity: 0; }
      10% { opacity: 1; }
      90% { opacity: 1; }
      to { transform: translateY(-100vh) scale(1.2); opacity: 0; }
    }

    @media (max-width: 400px) { .card { padding: 40px 20px; width: 85%; } }
    @keyframes pulse-green {
      0% { transform: scale(0.95); box-shadow: 0 0 0 0 rgba(76, 175, 80, 0.7); }
      70% { transform: scale(1); box-shadow: 0 0 0 6px rgba(76, 175, 80, 0); }
      100% { transform: scale(0.95); box-shadow: 0 0 0 0 rgba(76, 175, 80, 0); }
    }
    @keyframes float { 0%, 100% { transform: translateY(0); } 50% { transform: translateY(-10px); } }
    @keyframes drift { 0% { transform: translate(0, 0); } 100% { transform: translate(30px, 20px); } }
  </style>
</head>
<body>
  <div class="orb orb-1"></div>
  <div class="orb orb-2"></div>
  
  <div class="star-container" id="starContainer"></div>

  <div class="card">
    <img class="nut-img" src="data:image/webp;base64,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" alt="Walnut">

    <h1>The Walnut</h1>
    <div class="badge">System Online</div>

    <div class="info-group">
      <div class="info-item"><span class="label">Host Species</span><span class="value">Juglans regia</span></div>
      <div class="info-item">
        <span class="label">Status</span>
        <span class="value"><div class="status-dot"></div> Active</span>
      </div>
      <div class="info-item"><span class="label">Architecture</span><span class="value">RISC-V (ESP32-C6)</span></div>
    </div>

    <button id="spaceBtn" class="space-btn">Space 🚀</button>

    <p class="footer-note">The world's smallest standalone server, entirely inside a walnut shell.</p>
  </div>

  <script>
    console.log("Welcome to the Walnut Server.");
    
    // Generate Stars via JS to save memory size in the C++ string
    const starContainer = document.getElementById('starContainer');
    const starCount = 50;
    
    for(let i=0; i<starCount; i++) {
      let star = document.createElement('div');
      star.className = 'star';
      // Random Horizontal Position
      star.style.left = Math.random() * 100 + '%';
      // Random Vertical Start (spread them out initially)
      star.style.top = Math.random() * 100 + '%';
      // Random Size
      let size = Math.random() * 2 + 1; // 1px to 3px
      star.style.width = size + 'px';
      star.style.height = size + 'px';
      // Random Speed
      star.style.animationDuration = (Math.random() * 3 + 2) + 's'; // 2s to 5s
      star.style.animationDelay = (Math.random() * 5) + 's';
      starContainer.appendChild(star);
    }

    // Handle Hover Events
    const btn = document.getElementById('spaceBtn');
    const body = document.body;

    btn.addEventListener('mouseenter', () => {
      body.classList.add('space-active');
    });

    btn.addEventListener('mouseleave', () => {
      body.classList.remove('space-active');
    });
  </script>
</body>
</html>
)rawliteral";


void handleRoot() {
  server.send_P(200, "text/html", index_html);
}

void handleNotFound() {
  server.send(404, "text/plain", "404: The Nut is confused.");
}

void setup() {
  // reduce CPU frequency for battery life
  setCpuFrequencyMhz(80);
  Serial.begin(115200);

  Serial.println("\n--- Starting 2026 Walnut Server ---");

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // reduce WiFi power for battery life
  WiFi.setTxPower(WIFI_POWER_MINUS_1dBm);

  // put RF to sleep between router beacons for battery life
  WiFi.setSleep(true);

  Serial.println("\nConnected!");
  Serial.print("IP Address: http://");
  Serial.println(WiFi.localIP());

  if (!MDNS.begin(hostname)) {
    Serial.println("Error setting up MDNS responder!");
  } else {
    Serial.print("Access via Name: http://");
    Serial.println(String(hostname) + ".local");
  }

  server.on("/", handleRoot);
  server.onNotFound(handleNotFound);
  server.begin();
}

void loop() {
  server.handleClient();
  delay(500); 
}


