using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading;
using System.Windows;

using Translator;

namespace ClientX
{
    /// <summary>
    /// Interaction logic for LoginWindow.xaml
    /// </summary>
    public partial class LoginWindow : Window
    {
        public LoginWindow()
        {
            InitializeComponent();
        }

        /// <summary>
        /// Event handler function for the Login button click event
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void Button_Login_Click(object sender, RoutedEventArgs e)
        {
            // Set the shared property fields to their default values
            Application.Current.Properties["IsAdmin"] = false;
            Application.Current.Properties["Fullname"] = "He who must not be named";

            // Create a message with the username and password
            // And post it to the server
            Message message = new Message();
            EndPoint source = new EndPoint();
            source.Address = "localhost";
            source.Port = 10000;
            EndPoint destination = new EndPoint();
            destination.Address = "localhost";
            destination.Port = 9000;

            message.Source = source;
            message.Destination = destination;
            message.Type = (MessageType)MESSAGE_TYPE.USER_AUTHENTICATE;
            message.Body = string.Format("username:{0},password:{1}", Username.Text, Password.Password);
            message.ContentLength = message.Body.Length;
            Communication communication = (Communication)Application.Current.Properties["Communication"];
            communication.DeliverMessage(message);

            int attempts = 0;

            // Wait for a minute for the server to respond.
            // The IncomingMessageHandler processes authentication response
            // The ProcessMessage thread reads messages and sets shared property field
            // to indicate authentication status
            while (!Application.Current.Properties.Contains("IsAuthenticated") && ++attempts < 60)
            {
                Thread.Sleep(1000);
            }

            // If the shared field wasn't set, chances are the server didn't respond
            if (!Application.Current.Properties.Contains("IsAuthenticated"))
            {
                MessageBox.Show("Couldn't authenticate user. Possiblly due to trouble connecting to server");
                return;
            }

            // If the shared field is set, but set to false, don't allow user
            if(!(bool)Application.Current.Properties["IsAuthenticated"])
            {
                MessageBox.Show("Couldn't authenticate user. Invalid credentials, or possibly trouble connecting to server");
                return;
            }

            // Use is authenticated
            else
            {
                // If UserManagement was selected, and if user is admin, then show the management page
                // Else display a dialog and take user to the Test Harness page.
                Window window;
                if (CheckBox_UserManagement.IsChecked == null || !CheckBox_UserManagement.IsChecked.Value || !(bool)Application.Current.Properties["IsAdmin"])
                {
                    if (CheckBox_UserManagement.IsChecked != null && CheckBox_UserManagement.IsChecked.Value && !(bool)Application.Current.Properties["IsAdmin"])
                        MessageBox.Show("You're not an administrator. Taking you to test harness page instead");
                    window = new TestHarnessWindow();
                    window.Title = "TestHarness Client - " + Application.Current.Properties["Fullname"].ToString();
                    if ((bool)Application.Current.Properties["IsAdmin"])
                        window.Title += "(Admin)";
                    window.Show();
                }
                else
                {
                    window = new AddUserWindow();
                }
                window.Show();
                this.Close();
            }
        }

        /// <summary>
        /// Fires off a thread that handles incoming messages.
        /// The thread runs in the background, so once created, all pages
        /// need only access the message queue to see responses from server
        /// </summary>
        private void HandleIncomingMessages()
        {
            ThreadStart thrdProc = () => {
                while (true)
                {
                    // Use the shared communication channel to check for messages
                    Communication communication = (Communication)Application.Current.Properties["Communication"];

                    // This is a blocking call
                    Message incoming = communication.CollectMessage();
                    try
                    {
                        // If it's an authentication response, handle it and set shared properties
                        if((MESSAGE_TYPE)incoming.Type == MESSAGE_TYPE.USER_AUTHENTICATE)
                        {
                            if(!string.IsNullOrEmpty(incoming.Body))
                            {
                                if (incoming.Body.ToLower().Contains("isadmin:1"))
                                    Application.Current.Properties["IsAdmin"] = true;
                                Application.Current.Properties["Fullname"] = incoming.Body.Replace("authenticated:1", "").Replace(",isadmin:1", "").Replace(",fullname:","");
                                Application.Current.Properties["IsAuthenticated"] = incoming.Body.ToLower().Contains("authenticated:1");
                            }
                        }

                        // If it's a file list, populate a shared property
                        if((MESSAGE_TYPE)incoming.Type == MESSAGE_TYPE.FILE_LIST)
                        {
                            if(!string.IsNullOrEmpty(incoming.Body))
                            {
                                List<string> directories = new List<string>();
                                List<string> files = new List<string>();
                                foreach (string inode in incoming.Body.Split(new string[] { ","}, StringSplitOptions.RemoveEmptyEntries))
                                {
                                    if (inode.StartsWith("directory:") && !inode.Equals("directory:.") && !inode.Equals("directory:.."))
                                        directories.Add(inode.Replace("directory:", ""));
                                    else if (inode.StartsWith("file:"))
                                        files.Add(inode.Replace("file:", ""));
                                }
                                Application.Current.Properties["Directories"] = directories;
                                Application.Current.Properties["Files"] = files;
                                Application.Current.Properties["ListBoxesPopulated"] = true;
                            }
                        }

                        // If it's the server acknowledging a test request, just display it
                        if((MESSAGE_TYPE)incoming.Type == MESSAGE_TYPE.TEST_ACKNOWLEDGE)
                        {
                            MessageBox.Show("Server acknowledged test request: " + incoming.MessageId + ". Results will be available at /TESTRESULTS/" + incoming.MessageId + ".txt");
                        }
                    }
                    catch
                    {
                    }
                }
            };

            // Keep the thread running in the background
            incomingThread = new Thread(thrdProc);
            incomingThread.IsBackground = true;
            incomingThread.Start();
        }

        private Thread incomingThread;

        public enum MESSAGE_TYPE { FILE_GET, FILE_LIST, FILE_MKDIR, FILE_POST, SIGNAL_READY, SIGNAL_SHUTDOWN, TEST_ACKNOWLEDGE, TEST_EXECUTE, TEST_REQUEST, USER_AUTHENTICATE, USER_REGISTER };

        /// <summary>
        /// Event handler for the window loaded event
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void Window_Loaded(object sender, RoutedEventArgs e)
        {
            // Create a new communication channel by placing it in the shared properties.
            // This way, once we start the channel, all pages can use this channel to send
            // and receive messages
            Application.Current.Properties["Communication"] = new Communication(10000, 1);
            Communication communication = (Communication)Application.Current.Properties["Communication"];
            communication.StartListening();
            HandleIncomingMessages();
        }
    }
}
