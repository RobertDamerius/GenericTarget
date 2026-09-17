% This script starts a TCP echo server in MATLAB that receives one byte and send
% it back to the client. The server runs asynchronously in the background.
% To close the server, clear the "server" variable.
% 
% NOTES (R2025b)
% 
% (1) The TCP server only accepts one client. Once the client disconnects the TCP
%     server never accepts any client again.
% 
% (2) Once the server is closed it may lasts up to a few minutes to clear all
%     resources and allow a new server start.

server = tcpserver("::", 4000);
configureCallback(server, "byte", 1, @echoCallback);

function echoCallback(src, ~)
    data = read(src, 1, "uint8");
    write(src, data, "uint8");
    fprintf("[TCP Echo Server] 0x%02X\n", data);
end

