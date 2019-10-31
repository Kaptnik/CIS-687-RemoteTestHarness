using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;

using Translator;

namespace ClientX
{
    /// <summary>
    /// Interaction logic for TestHarnessWindow.xaml
    /// </summary>
    public partial class TestHarnessWindow : Window
    {
        private static string CURRENT_PATH = "/";
        private static string TOP_DIRECTORY = "[TOP DIRECTORY]";
        private static string XML_TEMPLATE =
            @"
            <Tests>
              <count>1</count>
              <item_version>0</item_version>
              <item>
                <TestDriver>TEST_DRIVER</TestDriver>
                <TestedCodes>
                  <count>TESTED_CODE_COUNT</count>
                  <item_version>0</item_version>
                  TESTED_CODE_ITEMS
                </TestedCodes>
              </item>
            </Tests>
            ";
        private static string TEST_DRIVER;
        private static List<string> TESTED_CODE = new List<string>();
        public TestHarnessWindow()
        {
            InitializeComponent();
        }

        /// <summary>
        /// Event handler for the new folder button click event
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void NewFolder_Button_Click(object sender, RoutedEventArgs e)
        {
            // Display a popup text box to get a folder name
            // This is available easily in C#, so I had to use a VB forms object
            // *tsk tsk*, I'm not proud it.
            string folderName = Microsoft.VisualBasic.Interaction.InputBox("Enter folder name", "Create Folder", "New Folder");

            // Use the shared communication channel and send the server a message
            Communication communication = (Communication)Application.Current.Properties["Communication"];
            Message message = new Message();
            message.Source = new EndPoint();
            message.Source.Address = "localhost";
            message.Source.Port = 10000;
            message.Destination = new EndPoint();
            message.Destination.Address = "localhost";
            message.Destination.Port = 9000;
            message.Type = (MessageType)LoginWindow.MESSAGE_TYPE.FILE_MKDIR;
            message.Body = CURRENT_PATH + "/" + folderName;
            message.ContentLength = message.Body.Length;

            communication.DeliverMessage(message);

            // Wait for the server to receive and process request
            Thread.Sleep(1000);

            // Clear and refresh the listboxes to display changes
            Clear_ListBoxes();
            Populate_ListBoxes();
        }

        /// <summary>
        /// Event handler for the window loaded event
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void Window_Loaded(object sender, RoutedEventArgs e)
        {
            // Clear the list boxes
            Clear_ListBoxes();
            // Populate them again
            Populate_ListBoxes();

            // Disable all buttons as the page has just loaded
            TestDriverButton.IsEnabled = false;
            TestedCodeButton.IsEnabled = false;
            ExecuteButton.IsEnabled = false;
        }

        /// <summary>
        /// Function that populates the listboxes
        /// </summary>
        private void Populate_ListBoxes()
        {
            // Use the shared communication channel to get list of 
            // files and directories from the server
            Communication communication = (Communication)Application.Current.Properties["Communication"];
            Message message = new Message();
            message.Source = new EndPoint();
            message.Source.Address = "localhost";
            message.Source.Port = 10000;
            message.Destination = new EndPoint();
            message.Destination.Address = "localhost";
            message.Destination.Port = 9000;
            message.Type = (MessageType)LoginWindow.MESSAGE_TYPE.FILE_LIST;
            message.Body = CURRENT_PATH;
            message.ContentLength = message.Body.Length;

            communication.DeliverMessage(message);

            // Sleep until the server responds
            while (!Application.Current.Properties.Contains("ListBoxesPopulated") || !(bool)Application.Current.Properties["ListBoxesPopulated"])
            {
                Thread.Sleep(500);
            }

            // Set the current path
            Path.Content = CURRENT_PATH;

            // Populate the files and directories
            // Always add a "[TOP DIRECTORY]" at the top of the directory listing
            // to navigate one level above
            // Navigate into directories by double clicking on them
            List<string> directories = (List<string>)Application.Current.Properties["Directories"];
            List<string> files = (List<string>)Application.Current.Properties["Files"];
            DirectoryList.Items.Add(new ListBoxItem() { Content = TOP_DIRECTORY });
            directories.ForEach(d => DirectoryList.Items.Add(new ListBoxItem() { Content = d }));
            files.ForEach(f => FileList.Items.Add(new ListBoxItem() { Content = f }));

            // Set populated to false so the next time a request is sent to the server,
            // the UI actually waits for a response and doesn't display stale data
            Application.Current.Properties["ListBoxesPopulated"] = false;
        }

        /// <summary>
        /// Just clear the list boxes. Clear as daylight
        /// </summary>
        private void Clear_ListBoxes()
        {
            Path.Content = "";
            DirectoryList.Items.Clear();
            FileList.Items.Clear();
        }

        /// <summary>
        /// Event handler for the directory double click event
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void DirectoryList_DoubleClick(object sender, MouseButtonEventArgs e)
        {
            // If the selected directory is the top directory
            // and we're already at the root folder (/), just return
            string directory = ((ListBoxItem)DirectoryList.SelectedValue).Content.ToString();
            if(directory.Equals(TOP_DIRECTORY))
            {
                if (CURRENT_PATH == "/")
                    return;
                // Else replace CURRENT_PATH by removing the last directory in it
                directory = CURRENT_PATH.Substring(0, CURRENT_PATH.Length - 1);
                directory = directory.Substring(0, directory.LastIndexOf('/'));
                CURRENT_PATH = directory + "/";
            }
            // If one of the sub folders was clicked, then append it to the CURRENT_PATH.
            else
                CURRENT_PATH += directory + "/";

            // Whatever the current path, clear and repopulate the listboxes
            Clear_ListBoxes();
            Populate_ListBoxes();
        }

        /// <summary>
        /// Event handler for the upload files button click event
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void UploadFiles_Button_Click(object sender, RoutedEventArgs e)
        {
            // Open up a file dialog to ask user to choose a file to upload
            Microsoft.Win32.OpenFileDialog fileDialog = new Microsoft.Win32.OpenFileDialog();
            fileDialog.Multiselect = false;

            Nullable<bool> result = fileDialog.ShowDialog();

            // If a file was selected
            if(result != null && result.Value)
            {
                string filename = fileDialog.FileName;

                // Use the shared communication channel and post the file to the server
                // Base64 encode the body (the server will always assume message body is base64 encoded)
                // to pass on binary files as strings
                Communication communication = (Communication)Application.Current.Properties["Communication"];
                Message message = new Message();
                message.Source = new EndPoint();
                message.Source.Address = "localhost";
                message.Source.Port = 10000;
                message.Destination = new EndPoint();
                message.Destination.Address = "localhost";
                message.Destination.Port = 9000;
                message.Type = (MessageType)LoginWindow.MESSAGE_TYPE.FILE_POST;
                message.Filename = CURRENT_PATH + System.IO.Path.GetFileName(filename);
                message.Body = Convert.ToBase64String(System.IO.File.ReadAllBytes(filename));
                message.ContentLength = message.Body.Length;

                communication.DeliverMessage(message);

                // Give the server a couple of seconds before refreshing the view
                Thread.Sleep(2000);
                Clear_ListBoxes();
                Populate_ListBoxes();
            }
        }

        /// <summary>
        /// Event handler for the download button click event
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void Download_Button_Click(object sender, RoutedEventArgs e)
        {
            // Just make sure that a file was selected before download was hit
            string selectedFilename = string.Empty;
            try
            {
                selectedFilename = ((ListBoxItem)FileList.SelectedValue).Content.ToString();
            }
            catch
            {
                return;
            }

            if (string.IsNullOrEmpty(selectedFilename))
                return;

            // Ask the user for a download location
            using (System.Windows.Forms.FolderBrowserDialog folderDialog = new System.Windows.Forms.FolderBrowserDialog())
            {
                System.Windows.Forms.DialogResult result = folderDialog.ShowDialog();

                // If a download location was chosen (as opposed to Cancel being hit)
                // then proceed to request the server for the file and download it
                if(result == System.Windows.Forms.DialogResult.OK)                
                {
                    string downloadFileName = folderDialog.SelectedPath + "/" + selectedFilename;
                    Communication communication = (Communication)Application.Current.Properties["Communication"];
                    Message message = new Message();
                    message.Source = new EndPoint();
                    message.Source.Address = "localhost";
                    message.Source.Port = 10000;
                    message.Destination = new EndPoint();
                    message.Destination.Address = "localhost";
                    message.Destination.Port = 9000;
                    message.Type = (MessageType)LoginWindow.MESSAGE_TYPE.FILE_GET;
                    message.Filename = downloadFileName;
                    message.Body = CURRENT_PATH + selectedFilename;
                    message.ContentLength = message.Body.Length;

                    communication.DeliverMessage(message);
                    Thread.Sleep(2000);
                    Clear_ListBoxes();
                    Populate_ListBoxes();
                }
            }
        }

        /// <summary>
        /// Event handler for the test driver button click event
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void TestDriver_Button_Click(object sender, RoutedEventArgs e)
        {
            // Make sure a file was selected
            string selectedFilename = string.Empty;
            try
            {
                selectedFilename = ((ListBoxItem)FileList.SelectedValue).Content.ToString();
            }
            catch
            {
                return;
            }
            // Set the test driver path, and disable the button
            TEST_DRIVER = CURRENT_PATH + selectedFilename;
            TestDriverButton.IsEnabled = false;
        }

        /// <summary>
        /// Event handler for the file selection changed event
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void FileList_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            // Ensure that a file was selected
            string selectedFilename = string.Empty;
            try
            {
                selectedFilename = ((ListBoxItem)FileList.SelectedValue).Content.ToString();
            }
            catch
            {
                return;
            }
            string extension = selectedFilename.Substring(selectedFilename.LastIndexOf('.')).ToLower();
            // If it's not a dll, then don't enable the test driver or tested code buttons
            if(!extension.Equals(".dll"))
            {
                TestDriverButton.IsEnabled = false;
                TestedCodeButton.IsEnabled = false;
            }
            // Similarly disable view button for non text/log/xml files
            if(!(new List<string>() { ".txt", ".log", ".xml" }.Contains(extension)))
            {
                ViewButton.IsEnabled = false;
            }
            // Else, enable the test driver button if it's not already set
            if (string.IsNullOrEmpty(TEST_DRIVER) && extension.Equals(".dll"))
                TestDriverButton.IsEnabled = true;
            // Enable the tested code button if the current file selected
            // isn't already selected as a tested code, or the test driver
            if (!TESTED_CODE.Contains(CURRENT_PATH + selectedFilename) && TEST_DRIVER != (CURRENT_PATH + selectedFilename) && extension.Equals(".dll"))
                TestedCodeButton.IsEnabled = true;

            // Enable execute button if at least one driver/tested code has been chosen
            CheckEnableExecutionButton();
        }

        /// <summary>
        /// Event handler for the tested code button click event
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void TestedCode_Button_Click(object sender, RoutedEventArgs e)
        {
            // Ensure a file was selected
            string selectedFilename = string.Empty;
            try
            {
                selectedFilename = ((ListBoxItem)FileList.SelectedValue).Content.ToString();
            }
            catch
            {
                return;
            }
            // Add it to the list of tested code files
            // Disable the button & check if we need to enable
            // the execute button
            TESTED_CODE.Add(CURRENT_PATH + selectedFilename);
            TestedCodeButton.IsEnabled = false;
            CheckEnableExecutionButton();
        }

        /// <summary>
        /// Event handler for the execute button click event
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void Execute_ButtonClick(object sender, RoutedEventArgs e)
        {
            // Customize the request xml to include 
            // the selected test driver & tested codes
            string requestXml = XML_TEMPLATE
                .Replace("TEST_DRIVER", TEST_DRIVER)
                .Replace("TESTED_CODE_COUNT", TESTED_CODE.Count.ToString())
                .Replace("TESTED_CODE_ITEMS", string.Join("\n", TESTED_CODE.Select(t => "<item>" + t + "</item>")));

            // Use the shared communication channel
            // to post the test request to the server
            Communication communication = (Communication)Application.Current.Properties["Communication"];
            Message message = new Message();
            message.Source = new EndPoint();
            message.Source.Address = "localhost";
            message.Source.Port = 10000;
            message.Destination = new EndPoint();
            message.Destination.Address = "localhost";
            message.Destination.Port = 9000;
            message.Type = (MessageType)LoginWindow.MESSAGE_TYPE.TEST_REQUEST;
            message.Body = requestXml;
            message.ContentLength = message.Body.Length;

            communication.DeliverMessage(message);

            // Reset the test driver & tested code
            TEST_DRIVER = string.Empty;
            TESTED_CODE.Clear();
            ExecuteButton.IsEnabled = false;
        }    


        /// <summary>
        /// Enable execute button if conditions are met
        /// </summary>
        private void CheckEnableExecutionButton()
        {
            // Ensure that we have a test driver and at least one test code file
            if (!string.IsNullOrEmpty(TEST_DRIVER) && TESTED_CODE.Count > 0)
            {
                ExecuteButton.IsEnabled = true;
            }
        }

        /// <summary>
        /// Event handler for the view button click event
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void View_Button_Click(object sender, RoutedEventArgs e)
        {
            // Ensure that a file was selected
            string selectedFilename = string.Empty;
            try
            {
                selectedFilename = ((ListBoxItem)FileList.SelectedValue).Content.ToString();
            }
            catch
            {
                return;
            }
            // Download the file locally to a temp location
            // The download is done very similar to the download button click event
            string downloadFileName = Guid.NewGuid().ToString() + "_" + selectedFilename;
            Communication communication = (Communication)Application.Current.Properties["Communication"];
            Message message = new Message();
            message.Source = new EndPoint();
            message.Source.Address = "localhost";
            message.Source.Port = 10000;
            message.Destination = new EndPoint();
            message.Destination.Address = "localhost";
            message.Destination.Port = 9000;
            message.Type = (MessageType)LoginWindow.MESSAGE_TYPE.FILE_GET;
            message.Filename = downloadFileName;
            message.Body = CURRENT_PATH + selectedFilename;
            message.ContentLength = message.Body.Length;

            communication.DeliverMessage(message);

            // Wait for the download to complete, for about 30s
            int attempts = 60;
            while(!System.IO.File.Exists(downloadFileName) && --attempts > 0)
            {
                Thread.Sleep(500);
            }
            // If this file still hasn't shown up, then show an error
            if(!System.IO.File.Exists(downloadFileName))
            {
                MessageBox.Show("Couldn't open file " + selectedFilename + " for viewing.");
                return;
            }
            // This sleep isn't necessary per say, but the 500ms should be enough
            // for the incoming message handler to write the file fully to disk
            Thread.Sleep(500);

            // Open up a text viewer, load the file contents in it, and show it
            TextViewer textViewer = new TextViewer();
            textViewer.TextArea.Text = System.IO.File.ReadAllText(downloadFileName);
            textViewer.Title = selectedFilename;
            textViewer.ResizeMode = ResizeMode.NoResize;
            textViewer.Show();
            // Delete the temp file, but don't fuss about it 
            // if it fails
            try
            {
                System.IO.File.Delete(downloadFileName);
            }
            catch
            {
            }
        }
    }
}
