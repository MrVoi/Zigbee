#Chep du lieu tu file nay vo file .bash trong ubuntu roi chay
#Hien test noi nhieu lenh trong alias:
alias test='sudo apt-get update && sudo subl'

#1. install CCS:
alias CCStool1='sudo apt-get install libc6-i386 libx11-6:i386 && sudo apt-get install libc6-i386 libasound2:i386 libjpeg62:i386 libatk1.0-0:i386 libcairo2:i386 libdbus-1-3:i386 libdbus-glib-1-2:i386 libfontconfig1:i386 libfreetype6:i386 libgconf-2-4:i386 libgdk-pixbuf2.0-0:i386 libgtk2.0-0:i386 libice6:i386 lib32ncurses5 liborbit2:i386 libpango-1.0-0:i386 libpangocairo-1.0-0:i386 libpangoft2-1.0-0:i386 libpng12-0:i386 libsm6:i386 lib32stdc++6 libusb-0.1-4:i386 libx11-6:i386 libxext6:i386 libxi6:i386 libxrender1:i386 libxt6:i386 libxtst6:i386 lib32z1 libgnomevfs2-0:i386 libcanberra-gtk-module:i386 && sudo apt-get install libusb-1.0-0 libusb-1.0-0:i386 libusb-1.0-0-dev libusb-1.0-0-dev:i386 && sudo apt-get install libcups2:i386 libgtk-3-0:i386 libncurses5:i386 libudev1:i386 libstdc++6:i386 libgnomeui-0:i386 libusb-1.0-0-dev:i386 && sudo apt-get install gtk2-engines-murrine:i386 && sudo apt-get install pinta'
alias install_CCS='cd Downloads/CCS60/CCS6.0.1.00040_linux && chmod +x ccs_setup_6.0.1.00040.bin && sudo ./ccs_setup_6.0.1.00040.bin'

#2. Open CCS in terminal
alias openCCS='cd /opt/ti/ccsv8/eclipse/ && sudo chmod u+x ccstudio && sudo ./ccstudio'

#3. Install package.deb
alias install_deb='sudo dpkg -i'

#4. Install open in terminal package:
alias open_in_terminal='sudo add-apt-repository universe && sudo apt-get update && sudo apt-get install nautilus-open-terminal && sudo apt-get install nautilus-action && nautilus -q'

#5. Cac goi tin nen cai trong LINUX/Ubuntu: https://kipalog.com/posts/Can-lam-nhung-gi-truoc--trong-va-sau-khi-cai-dat-Ubuntu
#5.1.msttcorefonts
alias msttcorefonts='sudo apt-get install ttf-mscorefonts-installer -y'
#5.2.Trinh tang toc download:
alias install_speeddownload='sudo add-apt-repository ppa:plushuang-tw/uget-stable && sudo apt-get update && sudo apt-get install uget && sudo apt-get install aria2'
#5.3.git:
alias install_git='sudo apt-get update && sudo apt-get install git'
alias config_git='git config --global user.name "Your Name" && git config --global user.email "youremail@domain.com"'

#5.4. Dọn dẹp file dư thừa
alias cleaning='sudo apt-get -f install && sudo apt-get autoremove -y && sudo apt-get autoclean -y && sudo apt-get clean -y'

#5.5. Install Unity Tweak Tool, Numix Theme và tùy chỉnh giao diện
alias unitytweaktool='sudo add-apt-repository ppa:numix/ppa -y && sudo apt-get update && sudo apt-get install numix-gtk-theme numix-icon-theme-circle -y && sudo apt-get install unity-tweak-tool -y'
#5.6. Synergy
alias install_synergy='sudo apt-get install synergy'
#5.7. Guake
alias install_guake='sudo apt-get install guake'
#5.8. Unikey:
alias install_unikey='sudo apt-get install ibus-unikey'

#6. Contikitool:
alias contiki_tools='sudo apt-get install build-essential binutils-msp430 gcc-msp430 msp430-libc msp430mcu mspdebug gcc-arm-none-eabi gdb-arm-none-eabi openjdk-8-jdk openjdk-8-jre ant libncurses5-dev lib32ncurses5 && sudo apt-get install automake && sudo apt-get install checkinstall'

#7. Bien dich phan mem ubuntu: http://www.ngohaibac.net/bien-dich-mot-goi-phan-mem-tren-ubuntu/
#8. Python: https://tecadmin.net/install-python-3-6-ubuntu-linuxmint/
alias install_toolpython3='sudo apt-get install build-essential checkinstall && sudo apt-get install libreadline-gplv2-dev  libncursesw5-dev libssl-dev libsqlite3-dev tk-dev libgdbm-dev libc6-dev libbz2-dev'
alias install_python3='cd /usr/src && sudo wget https://www.python.org/ftp/python/3.6.4/Python-3.6.4.tgz && sudo tar xzf Python-3.6.4.tgz'
alias compile_python_source='sudo apt-get install pip3 && cd Python-3.6.4 && sudo ./configure --enable-optimizations && sudo make altinstall'

#9. pdf:
alias install_foxit='wget http://cdn01.foxitsoftware.com/pub/foxit/reader/desktop/linux/2.x/2.1/en_us/FoxitReader2.1.0805_Server_x64_enu_Setup.run.tar.gz && tar xzvf FoxitReader*.tar.gz && sudo chmod a+x FoxitReader*.run && sudo ./FoxitReader.*.run'

#10. Install sublime-text-3:
alias install_sublime='wget -qO - https://download.sublimetext.com/sublimehq-pub.gpg | sudo apt-key add - && echo "deb https://download.sublimetext.com/ apt/stable/" | sudo tee /etc/apt/sources.list.d/sublime-text.list && sudo apt-get update && sudo apt-get install sublime-text'
alais unintall_sublime='sudo apt-get remove sublime-text && sudo apt-get autoremove'
