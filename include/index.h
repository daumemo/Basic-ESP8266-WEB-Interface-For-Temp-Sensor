const char MAIN_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<style>
.device-card{
    width: 300px;
    height: auto;
    display: inline-block;
    left: 50%;
    top: 50%;
    position: absolute;
    transform: translate(-50%, -50%);
    margin-bottom: 5px;
}

.device-control-main{
    width: 100%;
    height: 204px;
    display: inline-block;
    position: relative;
    border-radius: 8px;
    box-shadow: 0 1px 3px 0 rgba(0, 0, 0, 0.4);
    background-color: white;
}
.device-control-extra{
    width: 100%;
    height: 48px;
    display: inline-block;
    position: relative;
    border-radius: 8px;
    box-shadow: 0 1px 3px 0 rgba(0, 0, 0, 0.4);
    text-align: center;
    background-color: white;
}

.name{
    position: absolute;
    top:50%;
    left:50%;
    transform: translate(-50%, -50%);
    font-family: Roboto, sans-serif;
    font-size: 20px;
    font-weight:normal;
    color: lightgray;
    -webkit-user-select: none; /* Safari */
    -moz-user-select: none; /* Firefox */
    -ms-user-select: none; /* IE10+/Edge */
    user-select: none;
}

body{
    background-color: whitesmoke;
}

.hidden{
    display:none;
}

.temp-text{
    width: 100%;
    height: 50%;
    position: absolute;
    overflow: hidden;
    left: 0;
    top: 0;
    border-bottom: solid 1px #ddd;
}
.rh-text{
    width: 100%;
    height: 50%;
    position: absolute;
    overflow: hidden;
    left: 0;
    bottom: 0;
}
.value{
    position: absolute;
    width: 100%;
    text-align: center;
    top:50%;
    left:50%;
    transform: translate(-50%, -50%);
    font-family: Roboto, sans-serif;
    font-size: 40px;
    font-weight: lighter;
    color: lightgray;
    -webkit-user-select: none; /* Safari */        
    -moz-user-select: none; /* Firefox */
    -ms-user-select: none; /* IE10+/Edge */
    user-select: none;
}

</style>
</head>
<body>

<div class="device-card">
    <div class="device-control-extra">
        <div class="name">Temperature/RH</div>
    </div>
    <div class="device-control-main">
        <div class="temp-text"><div id="temp-value" class="value">0.00 C</div></div>
        <div class="rh-text"><div id="rh-value" class="value">50 %RH</div></div>
    </div>
</div>

<script>
    var temperature = 0.0;
    var relativeHumidity = 0;
    var periodicCheck;
    
    function getDataFromServer()
    {
        var xmlhttp = new XMLHttpRequest();
        xmlhttp.onreadystatechange = function() {
            if (this.readyState == 4 && this.status == 200) {
            var myObj = JSON.parse(this.responseText);
            temperature = parseFloat(myObj.temp);
            relativeHumidity = parseFloat(myObj.rh);
            updateView();
            }
    };
    
    xmlhttp.open("GET", "/data");
    xmlhttp.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
    xmlhttp.send();
    }

    function updateView(){
       document.getElementById("temp-value").innerHTML = temperature + " C";
       document.getElementById("rh-value").innerHTML = relativeHumidity + " %";
    }
    
    function startDataChecks(){
        periodicCheck = setInterval(getDataFromServer, 2500);
    }

    function stopDataChecks(){
        clearInterval(periodicCheck);
    }
    
    getDataFromServer();
    startDataChecks();
</script>

</body>
</html>
)=====";