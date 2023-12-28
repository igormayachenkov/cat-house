exports.processRequest = async function(socket, bodyBuffer){
    // PARSE and VERIFY REQUEST (body : Buffer)
    // tempA:7.50 tempTarget:7.00 heating:0 tempB:1.81
    const time = new Date().getTime()
    let success = false
    let sql = null
    let responce = ""

    const body  = bodyBuffer.toString('utf8', 0, 160)// limit body size
        .replace(/\r|\n/g," ")
    const ip = socket.remoteAddress?.replace(/^.*:/, '')
    try{
        const parts = body.split(" ")
        if(parts.length!=4) throw "wrong data"

        let data = {}

        readValue(parts[0], "tempA",     Number.parseFloat, data) 
        readValue(parts[1], "tempTarget",Number.parseFloat, data) 
        readValue(parts[2], "heating",   Number.parseInt,   data) 
        readValue(parts[3], "tempB",     Number.parseFloat, data) 

        let reason = 2;// Log reason (1:start, 2:timer, 3:change)

        // Save in the database
        sql = 
            `INSERT INTO sensors (time,ip,tempA,tempTarget,heating,tempB,reason) `
          + `VALUES(${time},'${ip}',${data.tempA},${data.tempTarget},${data.heating},${data.tempB},${reason})`

        // Set responce
        responce = "7"

        success = true
    }catch(e){
        // Incorrect request
        responce = "13"
        // Save the hacker
        sql = `INSERT INTO hackers (time,ip,body) VALUES(${time},'${ip}','${body}')`
    }

    // SAVE IN THE DATABASE
    try{
        await db.query(sql);
    }catch(e){
        console.log('db saving error, sql: '+sql)
    }

    //  WRITE RESPONSE
    socket.write(responce);

    // LOG REQUEST
    console.log(
        new Date(time).toISOString()+" "
        + ip.padStart(15, ' ') +"  "
        + body.toString()
        + (success ? (' => '+responce) : "")
    )
}


const readValue = (string, name, parseFun, data)=>{
    let parts = string.split(":")
    if(parts.length!=2) throw "w"
    if(name!=parts[0])  throw "w"
    let value = parseFun(parts[1])
    if(Number.isNaN(value)) throw "w"
    // Write to the data object
    data[name] = value
}



// socket.write("HTTP/1.1 200 OK\n");
// socket.write("Content-Type: text/html\n");
// socket.write("Connection: close\n");
// socket.write("\n");
// socket.write("<!DOCTYPE HTML>\n");
// socket.write("<html><body>\r\n");
// socket.write("<h1>hello</h1>\r\n");
// socket.write("</body></html>\r\n");

