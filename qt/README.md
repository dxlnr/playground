### Installation & Build

```bash
scons
```

When `moc` is not found, run

```bash
sudo apt install qt6-base-dev qt6-tools-dev

# error handling
sudo rm /usr/bin/moc 
sudo ln -s /usr/lib/qt6/libexec/moc /usr/bin/moc

sudo rm /usr/bin/qmake
sudo ln -s /usr/lib/qt6/bin/qmake /usr/bin/qmake
```
