var mode = 0;

function show_smartcam() {
    document.getElementById("reid_block").style.display = 'none'; 
    document.getElementById("smartcam_block").style.display = ''; 
    mode = 0;
}

function show_reid() {
    document.getElementById("smartcam_block").style.display = 'none'; 
    document.getElementById("reid_block").style.display = ''; 
    mode = 1;
}

function show_input_value(sel){
    if(sel.value == "File"){
        document.getElementById("input_value").style.display = ''; 
    } 
}
