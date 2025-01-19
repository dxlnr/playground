### Installation & Build

```bash
scons
```

When `moc` is not found, run

```bash
sudo apt install qt6-base-dev qt6-tools-dev
sudo ln -s /usr/lib/qt6/libexec/moc /usr/bin/moc
# If you get the following error: "ln: failed to create symbolic link '/usr/bin/moc': File exists" try:
sudo rm /usr/bin/moc 
```
