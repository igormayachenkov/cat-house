const net                   = require("node:net")
const { Database }          = require("./database.js")
const { verifyDatabase }    = require("./db-verification.js")
const { tables }            = require("./db-scheme.js")
const { processRequest }    = require("./domain.js")


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

        socket.on('data', async (body) => {
            
            //processRequest(socket, "tempA:7.50 tempTarget:7.00 heating:0 tempB:1.81")//data);
            processRequest(socket, body);

            socket.end();
            socket.destroy();
        });

        socket.on('error', (err) => {
            console.log('socket error:',err);
        });

        socket.on('close', () => {
            //console.log('close');
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
