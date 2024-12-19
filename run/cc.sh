killall server_config_center
nohup ./_build/cpp/bin/server_config_center -c "./test_assets/cc.ini" >> ./logs/cc.log &
