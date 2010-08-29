call buildit.cmd

plink -P 10022 root@localhost -pw "" killall -9 socket_plugin
pscp -scp -P 10022 -pw "" socket_plugin root@localhost:/media/internal
