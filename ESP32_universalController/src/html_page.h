
#ifndef INDEX_HTML_H
#define INDEX_HTML_H

const char index_html[] = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Settings Page</title>
  <style>
    body { background-color: #f0f8ff; font-family: Arial, sans-serif; text-align: center; }
    h1 { color: #4682b4; text-align: center; }
    form { 
      max-width: 400px; 
      margin: auto; 
      padding: 20px; 
      background-color: #e0ffff; 
      border: 2px solid #4682b4; 
      border-radius: 10px;
      box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);
    }
    input[type='text'], input[type='datetime-local'], input[type='color'], select, input[type='range'] { 
      width: calc(100% - 22px); 
      padding: 10px; 
      margin: 8px 0; 
      border: 1px solid #4682b4; 
      border-radius: 5px; 
    }
    input[type='checkbox'] { margin: 10px 0; }
    input[type='submit'] { 
      background-color: #32cd32; 
      color: white; 
      padding: 10px 15px; 
      border: none; 
      border-radius: 5px; 
      cursor: pointer; 
    }
    input[type='submit']:hover { background-color: #228b22; }
    #ambiLightColor { height: 50px; }
    hr { margin: 20px 0; border: 1px solid #4682b4; }
    .range-container {
      position: relative;
      width: 100%;
    }
    .range-label {
      position: absolute;
      top: -30px;
      left: 0;
      width: 100%;
      text-align: center;
    }
  </style>
</head>
<body>
  <h1>Налаштування системи</h1>
  <form id="configForm">
    <label for="ntpServer">Вибір сервера реального часу:</label>
    <select id="ntpServer" name="ntpServer">
      <option value="0">pool.ntp.org</option>
      <option value="1">time.google.com</option>
      <option value="2">time.windows.com</option>
    </select><br>

    <label for="timezone">Вибір часового поясу:</label>
    <select id="timezone" name="timezone">
      <option value="-12">UTC -12:00</option>
      <option value="-11">UTC -11:00</option>
      <option value="-10">UTC -10:00</option>
      <option value="-9">UTC -9:00</option>
      <option value="-8">UTC -8:00</option>
      <option value="-7">UTC -7:00</option>
      <option value="-6">UTC -6:00</option>
      <option value="-5">UTC -5:00</option>
      <option value="-4">UTC -4:00</option>
      <option value="-3">UTC -3:00</option>
      <option value="-2">UTC -2:00</option>
      <option value="-1">UTC -1:00</option>
      <option value="0">UTC 0:00</option>
      <option value="1">UTC +1:00</option>
      <option value="2">UTC +2:00</option>
      <option value="3">UTC +3:00</option>
      <option value="4">UTC +4:00</option>
      <option value="5">UTC +5:00</option>
      <option value="6">UTC +6:00</option>
      <option value="7">UTC +7:00</option>
      <option value="8">UTC +8:00</option>
      <option value="9">UTC +9:00</option>
      <option value="10">UTC +10:00</option>
      <option value="11">UTC +11:00</option>
      <option value="12">UTC +12:00</option>
    </select><br>
    
    <hr>

    <input type="checkbox" id="manualTime" name="manualTime">
    <label for="manualTime">Не синхронізувати час:</label><br><br>
    
    <label for="datetime">Встановити дату та час:</label>
    <input type="datetime-local" id="datetime" name="datetime"><br>
    
    <label for="ambiLightColor">Фонова підсвітка (Колір):</label>
    <input type="color" id="ambiLightColor" name="ambiLightColor"><br>
    
    <hr>
    <br>
    <div class="range-container">
      <div class="range-label" id="brightnessLabel">Яскравість: 50</div>
      <label for="brightness">Фонова підсвітка (яскравість):</label>
      <input type="range" id="brightness" name="brightness" min="0" max="255" value="50"><br>
    </div>
    
    <label for="ambiLight">Включити підсвітку:</label>
    <input type="checkbox" id="ambiLight" name="ambiLight"><br>
  </form>
  <script>
    const configForm = document.getElementById('configForm');
    
    const updateBrightnessLabel = () => {
      const brightnessInput = document.getElementById('brightness');
      const brightnessLabel = document.getElementById('brightnessLabel');
      brightnessLabel.textContent = `Яскравість: ${brightnessInput.value}`;
    };

    document.getElementById('brightness').addEventListener('input', updateBrightnessLabel);

    const loadSettings = () => {
      fetch('/settings')
        .then(response => response.json())
        .then(settings => {
          Object.keys(settings).forEach(key => {
            const element = document.querySelector(`[name=${key}]`);
            if (element) {
              if (element.type === 'checkbox') {
                element.checked = settings[key] === 'on';
              } else if (element.type === 'range') {
                element.value = settings[key];
                updateBrightnessLabel();
              } else {
                element.value = settings[key];
              }
            }
          });
        })
        .catch(error => {
          console.error('Error:', error);
        });
    };

    window.onload = loadSettings;

    const handleChange = (event) => {
      const formData = new FormData(configForm);
      const fieldName = event.target.name;
      const fieldValue = formData.get(fieldName);

      const data = { [fieldName]: fieldValue };

      fetch('/submit', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json'
        },
        body: JSON.stringify(data)
      })
      .then(response => response.text())
      .then(data => {
        console.log(`Settings saved for ${fieldName}:`, data);
      })
      .catch(error => {
        console.error('Error:', error);
      });
    };

    configForm.addEventListener('change', handleChange);
    configForm.addEventListener('submit', function(event) {
      event.preventDefault();
    });
  </script>
</body>
</html>

)rawliteral";

#endif