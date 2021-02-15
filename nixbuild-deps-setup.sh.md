Setup microsoft vscode, powershell and others windows dependencies on your debian box/host.
*This .sh/.md is not ready yet, I'm adding stuff when i have freetime*
#MAKE SURE YOU DID READ THOSE LINKS --- 
#https://docs.microsoft.com/en-us/dotnet/core/introduction#deployment-models 
#https://docs.microsoft.com/en-us/dotnet/core/install/linux-scripted-manual#scripted-install 
sudo apt install software-properties-common apt-transport-https curl
# get the asc key
curl -sSL https://packages.microsoft.com/keys/microsoft.asc | sudo apt-key add -

#add the repo to your /etc/apt/source.list.d/LIST.HERE.LIST -- IF ADD-APT-REPO dont work you can echo or manually add it.
sudo add-apt-repository "deb [arch=amd64] https://packages.microsoft.com/repos/vscode stable main"

# Download the Microsoft repository GPG keys
wget https://packages.microsoft.com/config/debian/10/packages-microsoft-prod.deb 

# Register the Microsoft repository GPG keys
sudo dpkg -i packages-microsoft-prod.deb

# Update the list of products and install code if you needs it.
sudo apt-get update && install dotnet-sdk-3.1 dotnet-sdk-2.1 code 

#installation script is also available.
https://dot.net/v1/dotnet-install.sh 
################# you can also use snap if you wish to
#Bash

sudo snap install dotnet-sdk --classic --channel=5.0

Next, register the dotnet command for the system with the snap alias command:
Bash

sudo snap alias dotnet-sdk.dotnet dotnet
#

