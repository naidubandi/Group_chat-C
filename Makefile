all:
	mkdir -p c__build__c
	gcc server.c -o c__build__c/server
	gcc client.c -o c__build__c/client
