instructions to run the program at the beginning of file serviceControl.c

# ServiceTool-Recruitment-

ServiceTool is a command line tool meant to monitor the status of linux services
and start and stop them easily.

Service statuses can be:
- Running
- Stopped
- Starting
- Stopping
- Failed

## Hints

Services can be started, stopped and monitored by using linux commands like
```
systemctl start <service name>
systemctl stop <service name>
systemctl is-active <service name>
```

---

### Software requirements
- The software shall load services names from a file "services.txt"
- The user shall start and stop a service easily (usability)
- The user shall see the realtime status of a service
  - Refresh rate < 1 second
- The software shall be executed from a terminal over SSH over low-bandwidth connection

You can edit services.txt files according to your PC installed services
to test the tool during developement.

## Info for recruitement

You can write this tool in any programming language,
try to complete the software as much as you can.

### I don't use linux :(
If you don't have linux and you have no intention to install it just for this project
you can do the same for windows, no problem...
