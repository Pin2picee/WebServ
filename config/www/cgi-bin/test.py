#!/usr/bin/env python3
import sys

HTML = """<!doctype html>
<html lang="fr">
<head>
<meta charset="UTF-8" />
<title>Animation CGI</title>
<style>
:root {
  --bg1: #111827;
  --bg2: #0ea5e9;
  --bg3: #8b5cf6;
  --fg:  #f8fafc;
}
* { box-sizing: border-box; }
body {
  margin: 0; min-height: 100vh; display: grid; place-items: center;
  color: var(--fg); font-family: "Inter", system-ui, -apple-system, sans-serif;
  background: radial-gradient(circle at 20% 20%, rgba(14,165,233,0.15), transparent 30%),
              radial-gradient(circle at 80% 30%, rgba(139,92,246,0.18), transparent 32%),
              radial-gradient(circle at 50% 80%, rgba(14,165,233,0.12), transparent 35%),
              var(--bg1);
  overflow: hidden;
}
.bg-wave {
  position: fixed; inset: 0; z-index: 0;
  background: linear-gradient(120deg, var(--bg2), var(--bg3));
  opacity: 0.20; filter: blur(60px);
  animation: drift 10s ease-in-out infinite alternate;
}
@keyframes drift {
  from { transform: translate(-4%, -2%) scale(1.02); }
  to   { transform: translate(4%, 2%) scale(1.05); }
}
.card {
  position: relative; z-index: 1;
  width: min(600px, 90vw); padding: 32px 28px;
  background: rgba(17,24,39,0.6); border: 1px solid rgba(255,255,255,0.08);
  border-radius: 18px; box-shadow: 0 20px 60px rgba(0,0,0,0.35), inset 0 1px 0 rgba(255,255,255,0.05);
  backdrop-filter: blur(12px);
}
h1 { margin: 0 0 14px; font-weight: 700; letter-spacing: 0.4px; }
p  { margin: 0 0 20px; color: rgba(248,250,252,0.8); line-height: 1.5; }

.pulse-wrap {
  width: 220px; height: 220px; margin: 0 auto; position: relative;
}
.pulse-core {
  position: absolute; inset: 50%; width: 28px; height: 28px;
  transform: translate(-50%,-50%);
  background: linear-gradient(135deg, var(--bg2), var(--bg3));
  border-radius: 50%; box-shadow: 0 10px 30px rgba(14,165,233,0.45);
  animation: blink 1.6s ease-in-out infinite;
}
.pulse-ring {
  position: absolute; inset: 50%; border: 2px solid rgba(255,255,255,0.35);
  border-radius: 50%; transform: translate(-50%,-50%);
  animation: ripple 2.2s ease-out infinite;
}
.pulse-ring.r2 { animation-delay: 0.6s; border-color: rgba(255,255,255,0.22); }
.pulse-ring.r3 { animation-delay: 1.2s; border-color: rgba(255,255,255,0.15); }

@keyframes ripple {
  0%   { width: 10px; height: 10px; opacity: 0.9; }
  70%  { opacity: 0.2; }
  100% { width: 220px; height: 220px; opacity: 0; }
}
@keyframes blink {
  0%, 100% { transform: translate(-50%,-50%) scale(1.0); box-shadow: 0 10px 30px rgba(14,165,233,0.45); }
  50%      { transform: translate(-50%,-50%) scale(1.15); box-shadow: 0 16px 40px rgba(139,92,246,0.5); }
}
</style>
</head>
<body>
<div class="bg-wave"></div>
<div class="card">
  <h1>Animation CGI</h1>
  <p>Servie par un script Python CGI. Fond en gradient animé et pulsation centrale.</p>
  <div class="pulse-wrap">
    <div class="pulse-ring r1"></div>
    <div class="pulse-ring r2"></div>
    <div class="pulse-ring r3"></div>
    <div class="pulse-core"></div>
  </div>
</div>
</body>
</html>
"""

sys.stdout.write("Status: 200 OK\r\n")
sys.stdout.write("Content-Type: text/html; charset=utf-8\r\n\r\n")
sys.stdout.write(HTML)