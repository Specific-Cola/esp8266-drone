#include <Arduino.h>


// 嵌入式HTML代码
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
<meta charset="UTF-8">
<title>双拨杆遥控器</title>
<style>
  canvas {
    border: 1px solid #000;
    background-color: #f3f3f3;
  }
  #dashboard {
    font-family: Arial, sans-serif;
    margin-top: 10px;
  }
</style>
</head>
<body>
<canvas id="joystick1" width="200" height="200"></canvas>
<canvas id="joystick2" width="200" height="200"></canvas>
<div id="dashboard">
  <p>Joystick 1 - X: <span id="xValue1">100</span>, Y: <span id="yValue1">100</span></p>
  <p>Joystick 2 - X: <span id="xValue2">100</span>, Y: <span id="yValue2">100</span></p>
</div>

<script>
  function createJoystick(canvasId, xValueId, yValueId) {
    var canvas = document.getElementById(canvasId);
    var ctx = canvas.getContext('2d');
    var xValue = document.getElementById(xValueId);
    var yValue = document.getElementById(yValueId);
    var joystickPosition = { x: 100, y: 100 };
    var centerPosition = { x: 100, y: 100 };
    var rect = canvas.getBoundingClientRect();
    var offsetX = rect.left;
    var offsetY = rect.top;
    var dragging = false;

    // 计算拨杆移动范围的函数
    function calculateBounds() {
      var minX = 20;
      var maxX = canvas.width - 20;
      var minY = 20;
      var maxY = canvas.height - 20;
      return { minX: minX, maxX: maxX, minY: minY, maxY: maxY };
    }

    // 绘制拨杆的函数
    function drawJoystick() {
      ctx.clearRect(0, 0, canvas.width, canvas.height);
      ctx.beginPath();
      ctx.arc(joystickPosition.x, joystickPosition.y, 20, 0, Math.PI * 2, true);
      ctx.fillStyle = 'blue';
      ctx.fill();
      ctx.stroke();
    }

    // 更新拨杆位置的函数
    function moveJoystick(event) {
      if (dragging) {
        var bounds = calculateBounds();
        var x, y;
        if (event.touches) { // 处理触摸事件
          x = event.touches[0].clientX - offsetX;
          y = event.touches[0].clientY - offsetY;
        } else { // 处理鼠标事件
          x = event.clientX - offsetX;
          y = event.clientY - offsetY;
        }
        if (x < bounds.minX) {
          x = bounds.minX;
        } else if (x > bounds.maxX) {
          x = bounds.maxX;
        }
        if (y < bounds.minY) {
          y = bounds.minY;
        } else if (y > bounds.maxY) {
          y = bounds.maxY;
        }
        joystickPosition.x = x;
        joystickPosition.y = y;
        drawJoystick();
        // 更新仪表盘
        xValue.textContent = joystickPosition.x;
        yValue.textContent = joystickPosition.y;
        // 发送位置信息到服务器
        sendPosition(canvasId, joystickPosition.x, joystickPosition.y);
      }
    }

    // 触摸/鼠标点击开始事件
    function startEvent(event) {
      var bounds = calculateBounds();
      var x, y;
      if (event.touches) { // 处理触摸事件
        x = event.touches[0].clientX - offsetX;
        y = event.touches[0].clientY - offsetY;
      } else { // 处理鼠标事件
        x = event.clientX - offsetX;
        y = event.clientY - offsetY;
      }
      if (x >= bounds.minX && x <= bounds.maxX && y >= bounds.minY && y <= bounds.maxY) {
        dragging = true;
        moveJoystick(event);
        event.preventDefault();
      }
    }

    // 触摸/鼠标移动事件
    function moveEvent(event) {
      if (dragging) {
        moveJoystick(event);
        event.preventDefault();
      }
    }

    // 触摸/鼠标结束事件
    function endEvent(event) {
      dragging = false;
      joystickPosition.x = centerPosition.x;
      joystickPosition.y = centerPosition.y;
      drawJoystick();
      // 更新仪表盘
      xValue.textContent = joystickPosition.x;
      yValue.textContent = joystickPosition.y;
      // 发送位置信息到服务器（归位时的位置信息）
      sendPosition(canvasId, joystickPosition.x, joystickPosition.y);
    }

    canvas.addEventListener('touchstart', startEvent);
    canvas.addEventListener('mousedown', startEvent);
    document.addEventListener('touchmove', moveEvent);
    document.addEventListener('mousemove', moveEvent);
    document.addEventListener('touchend', endEvent);
    document.addEventListener('mouseup', endEvent);

    // 初始化拨杆位置
    drawJoystick();
  }

  // 创建两个拨杆
  createJoystick('joystick1', 'xValue1', 'yValue1');
  createJoystick('joystick2', 'xValue2', 'yValue2');

// 发送位置信息到服务器的函数
function sendPosition(canvasId, x, y) {
var xhr = new XMLHttpRequest();
xhr.open("GET", "/position?canvasId=" + canvasId + "&x=" + x + "&y=" + y, true);
xhr.send();
}
</script>

</body>
</html>


)rawliteral";