# CIS-687-RemoteTestHarness
A project written as part of the course CIS-687, Object Oriented Design, as part of the MS in CS program in Syracuse University

# Project Requirements

The project requirements are listed here.

https://ecs.syr.edu/faculty/fawcett/handouts/CSE687-OnLine/SynchronousLectures/Project4.htm

# Overview

There are three main executables in this project

1. Server.exe
2. GUIClient.exe
3. Worker.exe

After compiling (either Debug or Release), it's best to navigate to the folder and run the applications from there.

First, run `Server.exe`. It'll spawn two Worker.exe processes and show that they registered.
Then, run `GUIClient.exe`.

If you run `GUIClient.exe` by itself, it'll start up and display a login screen. You can keep this open until the server
is up and running before entering the credentials. If you do enter some credentials before the server comes up,
the GUI waits for up to a minute before returning the controls back to the user. Without a spinning wheel to indicate
processing, it might appear that the GUI is stuck, but in reality it's just spinning and waiting for the server to respond.

The credentials are stored in a base64-encoded file called `authentication.txt` that is part of the package, and is co-located
in the same folder as the `Server.exe` binaries, or rather must be co-located with the `Server.exe` binaries.

If you decode the base64 string, you'd see that there is a default admin user with

| username | password     |
|----------|--------------|
| admin    | this!s$p@rta |

Use these credentials to login for the first time.

In the login screen, at the bottom, there is a check box that reads `Open User Management`.

Checking that box while logging in will allow you to add additional users. Note that a user once added, can't be deleted through the GUI. The only way to do it right now is to open the `authentication.txt` file in the server, and delete all the lines except for the first line (which is the default admin user).

# Features Completed

1. Support uploading files - albeit one at a time. Use the Upload button below the directory list to upload a file to the current directory.
2. Support creating remote directories - Use the `New Folder` button below the directory list to create a new directory.
3. Support navigating/displaying remote Mock Repository.
4. Support browsing remote logs - Select a log file (from the TESTRESULTS folder, usually after tests are executed, or you could copy-paste/upload a file) and hit the 'View' button. This button should only
be enabled for files with extension .log, .txt & .xml.
5. Support selecting running tests on DLLs.

I think I need to elaborate a little on this one. The `Worker.exe` I have expects the test driver dll to implement an `ITest` interface.
I have provided you a sample DLL, `Test.dll`, that tests two other DLLs, `Vehicle.dll` & `Shape.dll`. It should be available in the `REPOSITORY/Test1` folder.

When executing a test, you need to select one test driver, and at least one tested code. For my Test.dll to succeed though, it needs both. You could test success/failure scenarios through selecting just one/both. In order to execute the test, choose `Test.dll` in the files list, and then hit the `Test Driver` button. Then proceed to select both `Vehicle.dll` & `Shape.dll`, one at a time, and hit the `Tested Code` button. This should enable the `Execute` button, by virtue of having selected a test driver a tested code. Hit execute, and the server sends an acknowledgement, which also states where the test log will be available.

At this point, the server forwards the request to one of the workers, and has it execute the tests, and send the results back. I haven't implemented the case where the worker directly sends a pass/fail message to the GUI, so you'd have to manually browse through to the `/TESTRESULTS` folder and find the results there.

# Bonus Features Completed

1. Support users entering the system with a username and a password, and only a user with administrator privileges shall be able to create new users - as said before, start with the default admin user, and create users as needed.
2. Support downloading remote files to a local directory in the client. I don't have support for downloading entire directories, but the original requirement said 'Download a file OR a complete directory', so I went for the first option.
