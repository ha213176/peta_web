function run(){
    let gst = "";

    if(mode == 0){
        const input = document.getElementById('input_type');
        const width = document.getElementById('width1');
        const height = document.getElementById('height1');
        const output = document.getElementById('output');
        const smartcam_model = document.getElementById('smartcam_model');
        gst = "sudo smartcam ";

        // 輸入
        switch(input.value){
            case "#":
                return;
            case "usb":
                gst = gst + "-u 0 ";
                break;
            case "MIPI camera":
                gst = gst + "--mipi ";
                break;
            case "File":
                var textInput = document.getElementById('SCFileSrcInput');
                // gst = gst + "--file " + "../../Alley.nv12.h264" + " ";
                gst = gst + "--file " + textInput.value + " ";
                break;
        }
            gst = gst + "-i h264 -W 1920 -H 1080 -r 30 --target ";
        
        // 輸出
        switch(output.value){
            case "#":
                return;
            case "dp":
                gst = gst + "dp";
                break;
            case "rtsp":
                gst = gst + "rtsp";
                break;
            case "file":
                gst = gst + "file";
                break;
        }

        gst = gst + " --aitask " + smartcam_model.value;
    }
    else if(mode == 1){
        gst = "sudo aibox-reid ";
        
        var input = document.getElementById('reidSrc0');
        textInput = document.getElementById('reidText0');
        switch(input.value){
            case "RTSP":
                gst = gst + "-s " + textInput.value + " ";
                gst = gst + " -t rtsp -p 0 "
            break;
            case "FILE":
                gst = gst + "-s " + textInput.value + " ";
                gst = gst + " -t file -p 0 "
            break;
        }

        input = document.getElementById('reidSrc1');
        textInput = document.getElementById('reidText1');
        switch(input.value){
            case "#": break;
            case "RTSP":
                gst = gst + "-s " + textInput.value + " ";
                gst = gst + " -t rtsp -p 1 "
            break;
            case "FILE":
                gst = gst + "-s " + textInput.value + " ";
                gst = gst + " -t file -p 1 "
            break;
        }

        input = document.getElementById('reidSrc2');
        textInput = document.getElementById('reidText2');
        switch(input.value){
            case "#": break;
            case "RTSP":
                gst = gst + "-s " + textInput.value + " ";
                gst = gst + " -t rtsp -p 2 "
            break;
            case "FILE":
                gst = gst + "-s " + textInput.value + " ";
                gst = gst + " -t file -p 2 "
            break;
        }

        input = document.getElementById('reidSrc3');
        textInput = document.getElementById('reidText3');
        switch(input.value){
            case "#": break;
            case "RTSP":
                gst = gst + "-s " + textInput.value + " ";
                gst = gst + " -t rtsp -p 3 "
            break;
            case "FILE":
                gst = gst + "-s " + textInput.value + " ";
                gst = gst + " -t file -p 3 "
            break;
        }

    }

    // 送封包
    console.log(gst);
    var xhr = new XMLHttpRequest();
    xhr.open("POST", "/", true);
    xhr.setRequestHeader('Content-Type', 'text');
    xhr.send("GSTREAMER: "+gst+"\r\0");
}

function quit(){
    var xhr = new XMLHttpRequest();
    xhr.open("POST", "/", true);
    xhr.setRequestHeader('Content-Type', 'text');
    xhr.send("GSTREAMER: quit\r\0");
}