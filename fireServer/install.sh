#!/bin/bash
no_build=1
no_update=1
no_install=1
run_after=0
setup_apache=1
while getopts "bnir" OPTION; do
        case "$OPTION" in
                b) 
					echo -e "\e[91mno build\e[0m"
					no_build=0 ;;
                n) 
					echo -e "\e[91mno update\e[0m"
					no_update=0 ;;
                i) 
					echo -e "\e[91mno install\e[0m"
					no_install=0 ;;
                r) 
					echo -e "\e[32mrunning after install\e[0m"
					run_after=1 ;;
                a) 
					echo -e "\e[32not setting up apache[0m"
					setup_apache=0 ;;
        esac
done

set -ex

if [[ $no_update == 1 ]]; then
	{ echo -e "\e[36m\e[4m >Installing dependancies< \e[0m"; } 2> /dev/null
	sudo apt update
	sudo apt upgrade -y
fi

sudo apt install cmake git sqlite3 libssl-dev -y

if [[ $no_install == 1 ]]; then
	mkdir build -p
	cd build

#	{ echo -e "\e[36m\e[4m >Installing bcm2835< \e[0m"; } 2> /dev/null
#	wget http://www.airspayce.com/mikem/bcm2835/bcm2835-1.68.tar.gz
#	tar -xzvf bcm2835-1.68.tar.gz
#	cd bcm2835-1.68/
#	./configure
#	sudo make
#	sudo make install
#	cd ..

	{ echo -e "\e[36m\e[4m >Installing mqtt.c< \e[0m"; } 2> /dev/null
	git clone https://github.com/eclipse/paho.mqtt.c.git
	cd paho.mqtt.c
	git checkout v1.3.6

	cmake -Bbuild -H. -DPAHO_WITH_SSL=ON -DPAHO_BUILD_STATIC=ON -DCMAKE_POSITION_INDEPENDENT_CODE=ON -DPAHO_ENABLE_TESTING=OFF
	sudo env "PATH=$PATH" cmake --build build/ --target install
	sudo ldconfig
	cd ..
	cd ..
	sudo rm build -r
fi

if [[ $setup_apache == 1 ]]; then
	{ echo -e "\e[36m\e[4m Setting Up apache2 \e[0m"; } 2> /dev/null
	sudo apt install apache2 php7.3 php7.3-sqlite3 php7.3-curl # or newer

	source /etc/apache2/envvars

	cd apache_files
	sudo cp gateway.css single_page.php /var/www/html/
	echo "DirectoryIndex single_page.php" | sudo tee /var/www/html/.htaccess 1>/dev/null
	sudo mkdir /var/lib/fireiot
	sudo chown www-data:www-data /var/lib/fireiot # make sure php can acess the database folder.
	cd ..
fi


{ echo -e "\e[36m\e[4m >Creating Build folder in alarm_monitoring< \e[0m"; } 2> /dev/null
cd alarm_monitoring
mkdir build -p

if [[ $no_build == 1 ]]; then
    cd build
    cmake ..
    make
    { echo -e "\e[36m\e[4m >navigate to alarm_monitoring/build and run the program with 'sudo alarm_monitoring/alarm_monitoring' \e[0m"; } 2> /dev/null
fi




if [[ $run_after == 1 ]]; then
	cd GatewayRPI
	sudo ./GatewayRPI
fi

exit 0