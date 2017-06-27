
let html = document.getElementsByTagName("html")[0]

fetchs = (method,url,pdata,callback) => {
    let fdata = {method: method}
    if(method == "POST"){
        fdata['body'] = JSON.stringify(pdata)
        console.log(fdata.body)
    }
    fetch(url, fdata)
	.then(res => res.json())
	.then(data => {
        callback(data)
	})
}

window.onload = () => {
    fetchs("GET","/srt/V_for_Vendetta",null,(data) => {
        let dat = doT.template(html.innerHTML)
		html.innerHTML = dat(data)
    })
}

submit = () => {
    let boxs = document.getElementsByName("checkbox");
    let pdata = {"lines":[]};
    for(c in boxs){
        if(boxs[c].checked)
            pdata["lines"].push({"index":boxs[c].value});
    }
    fetchs("POST", "/srt/V_for_Vendetta", pdata, (data) => {
        console.log(data);
    })
    
}

