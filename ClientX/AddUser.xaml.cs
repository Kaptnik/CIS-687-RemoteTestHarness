using System.Windows;

using Translator;

namespace ClientX
{
    /// <summary>
    /// Interaction logic for AddUserWindow.xaml
    /// </summary>
    public partial class AddUserWindow : Window
    {
        public AddUserWindow()
        {
            InitializeComponent();
        }

        /// <summary>
        /// EventHandler for the RegisterUser button click event
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void RegisterUser_Button_Click(object sender, RoutedEventArgs e)
        {
            // Generate a new message
            // Use the communication channel shared in Application.Current.Properties
            // and post it to the server.
            Communication communication = (Communication)Application.Current.Properties["Communication"];
            Message message = new Message();
            EndPoint source = new EndPoint();
            source.Address = "localhost";
            source.Port = 10000;
            EndPoint destination = new EndPoint();
            destination.Address = "localhost";
            destination.Port = 9000;

            message.Source = source;
            message.Destination = destination;

            message.Type = (MessageType)LoginWindow.MESSAGE_TYPE.USER_REGISTER;
            message.Body = string.Format("username:{0},password:{1},isadmin:{2},fullname{3}", Username.Text, Password.Password, (IsAdmin.IsChecked == null || !IsAdmin.IsChecked.Value ? "0" : "1"), Fullname.Text);
            message.ContentLength = message.Body.Length;
            communication.DeliverMessage(message);

            // Once posted, go back to the login window
            LoginWindow loginWindow = new LoginWindow();
            loginWindow.Show();
            this.Close();
        }

        /// <summary>
        /// Event handler for the cancel button click event
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void Cancel_Button_Click(object sender, RoutedEventArgs e)
        {
            // If canceled, just go back to the login screen
            LoginWindow loginWindow = new LoginWindow();
            loginWindow.Show();
            this.Close();
        }
    }
}
