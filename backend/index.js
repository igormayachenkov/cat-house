const net                   = require("node:net")
const { Database }          = require("./database.js")
const { verifyDatabase }    = require("./db-verification.js")
const { tables }            = require("./db-scheme.js")


async function main(){
try{

    console.log("-----------------------------------------------------------------------");

    // LOAD CONFIG
    const configFilename = "./config.js"// "/etc/cat-house/config.js"
    console.log("CONFIG FILE: " +configFilename);
    global.gConfig = require(configFilename);

    // OPEN DATABASE
    global.db = new Database(gConfig.DATABASE.database)
    console.log('DATABASE: ' + db.getDatabaseName());
    await verifyDatabase(tables)

    // START SERVER
    const server = net.createServer((socket) => {
        let ip = socket.remoteAddress?.replace(/^.*:/, '') 
        console.log(
            new Date().toISOString()+" "
            + socket.remoteAddress?.replace(/^.*:/, '').padStart(15, ' ') 
        )

        socket.on('data', (data) => {
            console.log('data', data.toString());

            socket.write('7');

            // socket.write("HTTP/1.1 200 OK\n");
            // socket.write("Content-Type: text/html\n");
            // socket.write("Connection: close\n");
            // socket.write("\n");
            // socket.write("<!DOCTYPE HTML>\n");
            // socket.write("<html><body>\r\n");
            // socket.write("<h1>hello</h1>\r\n");
            // socket.write("</body></html>\r\n");

            socket.end();
            socket.destroy();
        });

        socket.on('close', () => {
            console.log('close');
        });

    });

    server.on('error', (err) => {
        throw err;
    });

    server.listen(gConfig.PORT, () => {
        console.log(`SERVER STARTED on port ${gConfig.PORT}`);
    }); 
}catch(e){
    console.log(e)
    process.exit(1)
}}

main();
