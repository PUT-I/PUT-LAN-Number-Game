START cmd /c "Client_Instance.exe 2> log_client1.log"
timeout 15
START cmd /c "Client_Instance.exe 2> log_client2.log"
