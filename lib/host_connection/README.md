# serial_host_connection
Host connection gate using uart protocol

Install
```bash
# macOS
brew install protobuf

# Python 3
pip3 install protobuf
```

Add remote
```bash
git remote add -f host_connection https://github.com/toanpi/serial_host_connection.git
```

Remove subtree
```bash
git rm host_connection
git commit
```

Clone remote
```bash
git read-tree --prefix=host_connection -u host_connection/main
```
Pull
```bash
git pull -s subtree host_connection main
```

Push
```bash
git subtree push --prefix=host_connection host_connection main
```

Merge
```bash
git merge -s ours --no-commit host_connection/main
```

If git read-tree --prefix=host_connection -u host_connection/main
error: Entry 'host_connection/README.md' overlaps with 'host_connection/README.md'.  Cannot bind.
Try:

```bash
git merge --squash -s recursive -Xsubtree --allow-unrelated-histories host_connection/main
```

Sync folder
```bash
xcopy .\host_connection\ ..\serial_host_connection\  /D /I /E /F /Y /H /R
xcopy .\serial_host_connection\ .\Boston_Project_Phase_2\host_connection\  /D /I /E /F /Y /H /R
xcopy .\serial_host_connection\ .\BostonBackpackFirmware\host_connection\  /D /I /E /F /Y /H /R
```

