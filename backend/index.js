const net = require("node:net")

const PORT = 8888

const server = net.createServer((socket) => {
    let ip = socket.remoteAddress?.replace(/^.*:/, '') 
    console.log(
        new Date().toISOString()+" "
        + socket.remoteAddress?.replace(/^.*:/, '').padStart(15, ' ') 
    )

    socket.on('data', (data) => {
        console.log('data', data.toString());

        socket.write('hello');

        // socket.write("HTTP/1.1 200 OK\n");
        // socket.write("Content-Type: text/html\n");
        // socket.write("Connection: close\n");
        // socket.write("\n");
        // socket.write("<!DOCTYPE HTML>\n");
        // socket.write("<html><body>\r\n");
        // socket.write("<h1>hello</h1>\r\n");
        // socket.write("</body></html>\r\n");

        socket.end();
    });

    socket.on('end', () => {
        console.log(new Date().toISOString()+' client disconnected');
    });
});

server.on('error', (err) => {
    throw err;
});

server.listen(PORT, () => {
    console.log(`server started on port ${PORT}`);
}); 