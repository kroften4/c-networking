all: client server showip producer_consumer

client:
	gcc ./krftn_test_client.c -o ./bin/client

server:
	gcc ./krftn_test_server.c -o ./bin/server

showip:
	gcc ./showip.c -o ./bin/showip

producer_consumer:
	gcc ./prod_cons.c -o ./bin/prod_cons

