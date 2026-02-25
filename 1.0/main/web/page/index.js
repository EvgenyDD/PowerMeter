"use strict";

var isRecording = false;
var recordedData = [];
var recordingStartTime = null;

let currentRequest = null;
let cnt_disconn = 0;

function req_rt_data() {
    if (currentRequest && currentRequest.readyState !== 4) {
        currentRequest.abort();
        cnt_disconn = cnt_disconn + 1;
        if (cnt_disconn > 4) {
            cnt_disconn = 4;
            document.getElementById('labelOffline').style.display = 'block';
        }
    }

    currentRequest = new XMLHttpRequest();
    currentRequest.onreadystatechange = function () {
        if (currentRequest.readyState == 4 && currentRequest.status == 200) {
            rt_data_cb(currentRequest.responseText);
            cnt_disconn = cnt_disconn - 1;
            if (cnt_disconn < -4) {
                cnt_disconn = -4;
                document.getElementById('labelOffline').style.display = 'none';
            }
        }
    }
    currentRequest.open("GET", "/api/rtd", true);// true for asynchronous 
    currentRequest.send(null);
}

function logAppend(m) {
    var newlineIndex = m.indexOf('\n');
    if (newlineIndex === -1) {
        appendSingleLine(m);
    } else {
        var firstLine = m.substring(0, newlineIndex);
        var rest = m.substring(newlineIndex + 1);
        appendSingleLine(firstLine);
        logAppend(rest);
    }
}

function clearCounters() {
    var xmlHttp = new XMLHttpRequest();
    xmlHttp.open("POST", "/clear", true); // true for asynchronous 
    xmlHttp.send(null);
}

function appendSingleLine(line) {
    if (line === "") return;
    const line_mod = line.replace(/ /g, '&nbsp;');

    var d = new Date();
    document.getElementById('messageLog').innerHTML += "<a style=\"color:blue;font-size:12px\">" +
        d.toLocaleTimeString('en-US', { hour12: false }) + "." + ('00' + d.getMilliseconds()).slice(-3) +
        ": </a><code style=\"font-size:12pxwhite-space:pre\">" +
        line_mod +
        "</code><br/>";

    var elem = document.getElementById('messageLog');
    elem.scrollTop = elem.scrollHeight;
}

function fmt(num) {
    let numStr = num.toString();

    let parts = numStr.split('.');
    let integerPart = parts[0];
    let fractionalPart = parts[1] || '';

    // Format integer part with spaces every 3 digits from the right
    let formattedInteger = integerPart.replace(/\B(?=(\d{3})+(?!\d))/g, ' ');

    // Format fractional part with apostrophes every 3 digits from the left
    let formattedFractional = '';
    if (fractionalPart) {
        for (let i = 0; i < fractionalPart.length; i += 3) {
            if (i > 0) formattedFractional += "'";
            formattedFractional += fractionalPart.substring(i, i + 3);
        }
    }

    return formattedFractional ?
        formattedInteger + '.' + formattedFractional :
        formattedInteger;
}

function rt_data_cb(resp) {
    const obj = JSON.parse(resp);

    document.querySelectorAll(".rtd_app").forEach(label => { label.innerHTML = fmt((obj.app).toFixed(6)) + ' W×h'; });
    document.querySelectorAll(".rtd_act").forEach(label => { label.innerHTML = fmt((obj.act).toFixed(6)) + ' W×h'; });
    document.querySelectorAll(".rtd_wf").forEach(label => { label.innerHTML = fmt((obj.wf).toFixed(3)) + ' W'; });
    document.querySelectorAll(".rtd_urms").forEach(label => { label.innerHTML = fmt((obj.urms).toFixed(1)) + ' V'; });
    document.querySelectorAll(".rtd_irms").forEach(label => { label.innerHTML = fmt((obj.irms).toFixed(6)) + ' A'; });
    document.querySelectorAll(".rtd_freq").forEach(label => { label.innerHTML = fmt((obj.freq).toFixed(3)) + ' Hz'; });
    document.querySelectorAll(".rtd_temp").forEach(label => { label.innerHTML = fmt((obj.temp).toFixed(0)) + ' °C'; });
    document.querySelectorAll(".rtd_raw_app").forEach(label => { label.innerHTML = fmt((obj.raw_app).toFixed(0)); });
    document.querySelectorAll(".rtd_raw_act").forEach(label => { label.innerHTML = fmt((obj.raw_act).toFixed(0)); });
    document.querySelectorAll(".rtd_raw_wf").forEach(label => { label.innerHTML = fmt((obj.raw_wf).toFixed(0)); });
    document.querySelectorAll(".rtd_raw_urms").forEach(label => { label.innerHTML = fmt((obj.raw_urms).toFixed(0)); });
    document.querySelectorAll(".rtd_raw_irms").forEach(label => { label.innerHTML = fmt((obj.raw_irms).toFixed(0)); });
    document.querySelectorAll(".rtd_raw_temp").forEach(label => { label.innerHTML = fmt((obj.raw_temp).toFixed(0)); });
    document.querySelectorAll(".rtd_acc_app").forEach(label => { label.innerHTML = fmt((obj.acc_app).toFixed(0)); });
    document.querySelectorAll(".rtd_acc_act").forEach(label => { label.innerHTML = fmt((obj.acc_act).toFixed(0)); });
    document.querySelectorAll(".rtd_acc_react").forEach(label => { label.innerHTML = fmt((obj.acc_react).toFixed(0)); });
    document.querySelectorAll(".rtd_acc_time").forEach(label => { label.innerHTML = fmt((obj.acc_time).toFixed(0)); });
    document.querySelectorAll(".rtd_2nd_e").forEach(label => { label.innerHTML = fmt((obj.ext_e).toFixed(6)) + ' W×h'; });
    document.querySelectorAll(".rtd_2nd_p").forEach(label => { label.innerHTML = fmt((obj.ext_p).toFixed(3)) + ' W'; });
    document.querySelectorAll(".rtd_2nd_u").forEach(label => { label.innerHTML = fmt((obj.ext_u).toFixed(1)) + ' V'; });
    document.querySelectorAll(".rtd_2nd_i").forEach(label => { label.innerHTML = fmt((obj.ext_i).toFixed(6)) + ' A'; });

    if (obj.hasOwnProperty("console")) {
        logAppend(obj.console);
    }

    if (isRecording) {
        const timestamp = Date.now() - recordingStartTime;
        const record = {
            timestamp_ms: timestamp,
            raw_app: obj.raw_app,
            raw_act: obj.raw_act,
            raw_wf: obj.raw_wf,
            raw_urms: obj.raw_urms,
            raw_irms: obj.raw_irms,
            acc_app: obj.acc_app,
            acc_act: obj.acc_act,
            acc_react: obj.acc_react,
            acc_time: obj.acc_time,
            ext_p: obj.ext_p,
            ext_e: obj.ext_e,
            ext_u: obj.ext_u,
            ext_i: obj.ext_i
        };
        recordedData.push(record);
    }
}

function startRecording() {
    isRecording = true;
    recordedData = [];
    recordingStartTime = Date.now();
    console.log('Recording started');
}

function stopRecording() {
    isRecording = false;
    recordingStartTime = null;
    console.log(`Recording stopped. Captured ${recordedData.length} samples`);
}

function saveRecordedData() {
    if (recordedData.length === 0) {
        alert('No recorded data to save');
        return;
    }

    const headers = Object.keys(recordedData[0]);
    const csvContent = [
        headers.join(','),
        ...recordedData.map(row => headers.map(header => row[header]).join(','))
    ].join('\n');

    const blob = new Blob([csvContent], { type: 'text/csv;charset=utf-8;' });
    const link = document.createElement('a');
    const url = URL.createObjectURL(blob);

    const date = new Date();
    const filename = `recording_EM_${date.getFullYear()}${(date.getMonth() + 1).toString().padStart(2, '0')}${date.getDate().toString().padStart(2, '0')}_${date.getHours().toString().padStart(2, '0')}${date.getMinutes().toString().padStart(2, '0')}${date.getSeconds().toString().padStart(2, '0')}.csv`;

    link.href = url;
    link.setAttribute('download', filename);
    document.body.appendChild(link);
    link.click();
    document.body.removeChild(link);
    URL.revokeObjectURL(url);

    console.log(`Saved ${recordedData.length} samples to ${filename}`);
}

function consoleSend() {
    var xmlHttp = new XMLHttpRequest();
    xmlHttp.open("POST", "/console/" + document.getElementById("console").value, true); // true for asynchronous 
    xmlHttp.send(null);
    logAppend("send: " + document.getElementById("console").value);
}

function consoleSend2() {
    var xmlHttp = new XMLHttpRequest();
    xmlHttp.open("POST", "/console/" + document.getElementById("console2").value, true); // true for asynchronous 
    xmlHttp.send(null);
    logAppend("send: " + document.getElementById("console2").value);
}

function consoleSendEvent(event) {
    if (event.key === "Enter") {
        consoleSend()
    }
}
function consoleSendEvent2(event) {
    if (event.key === "Enter") {
        consoleSend2()
    }
}

function getRecordingStatus() {
    return {
        isRecording: isRecording,
        sampleCount: recordedData.length,
        duration: recordingStartTime ? Date.now() - recordingStartTime : 0
    };
}

function startUpload() {
    document.getElementById("status").innerHTML = "Downloading...";
    var otafile = document.getElementById("otafile").files;
    var file = otafile[0];
    var xhr = new XMLHttpRequest();
    xhr.onreadystatechange = function () {
        if (xhr.readyState == 4) {
            if (xhr.status == 200) {
                document.getElementById("status").innerHTML = xhr.responseText;
            } else if (xhr.status == 0) {
                alert("Server closed the connection");
                location.reload();
            } else {
                alert(xhr.status + " Error!\n" + xhr.responseText);
                location.reload();
            }
        }
    };
    xhr.upload.onprogress = function (e) { document.getElementById("status").innerHTML = ((e.loaded / e.total * 100).toFixed(0)) + "%"; };
    xhr.open("POST", "/update", true);
    xhr.send(file);
}

function toggleDivs() {
    if (document.getElementById('devModeSwitch').checked) {
        document.getElementById('basic_ui').style.display = 'none';
        document.getElementById('div_upd').style.display = 'block';
        document.getElementById('ext_ui').style.display = 'block';
        document.getElementById('div_rec').style.display = 'block';
        document.getElementById('div_console').style.display = 'block';
        document.getElementById('messageLog').style.display = 'block';
    } else {
        document.getElementById('basic_ui').style.display = 'block';
        document.getElementById('div_upd').style.display = 'none';
        document.getElementById('ext_ui').style.display = 'none';
        document.getElementById('div_rec').style.display = 'none';
        document.getElementById('div_console').style.display = 'none';
        document.getElementById('messageLog').style.display = 'none';
    }
}

window.onload = function () {
    setInterval(req_rt_data, 250);
    setInterval(() => {
        const status = getRecordingStatus();
        const statusEl = document.getElementById('recordingStatus');
        if (statusEl) {
            statusEl.innerHTML = `Recording: ${status.isRecording ? 'ON' : 'OFF'} | Samples: ${status.sampleCount} | Duration: ${(status.duration / 1000).toFixed(1)}s`;
        }
    }, 100);
}

window.startRecording = startRecording
window.stopRecording = stopRecording
window.saveRecordedData = saveRecordedData
window.startUpload = startUpload;
window.consoleSend = consoleSend;
window.consoleSend2 = consoleSend2;
window.consoleSendEvent = consoleSendEvent;
window.consoleSendEvent2 = consoleSendEvent2;
window.toggleDivs = toggleDivs;
window.clearCounters = clearCounters;