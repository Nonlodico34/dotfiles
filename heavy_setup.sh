sudo apt-get install nala -y
echo "alias apt='nala'" >> ~/.bashrc
source ~/.bashrc
sudo apt update
sudo apt upgrade -y

sudo apt install bat -y
sudo apt install zoxide -y
sudo apt install tmux -y
sudo apt install eza -y
sudo apt install ranger -y
sudo apt install build-essential -y
sudo apt install ncdu -y
sudo apt install bpytop -y

# # install fzf
# cd ~
# git clone --depth 1 https://github.com/junegunn/fzf.git ~/.fzf
# ~/.fzf/install
# rm -rf ~/.fzf
