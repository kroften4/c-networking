all: client server

client:
	gcc ./krftn_test_client.c -o ./bin/client

server:
	gcc ./krftn_test_server.c -o ./bin/server
