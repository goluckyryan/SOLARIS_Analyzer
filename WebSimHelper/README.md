# Introduction

This is a web inteface for the HELIOS/SOLARIS simulation. Its purpose is NOT to replace the Simulation_Helper.C in the origin digios repository.
It is simply provide a more easy accessible way to do simulation.

# Installation in Apache2

Assume the parant SOLARIS_ANALYSIS is in the home folder
add a symbolic link

```sh
$cd /var/www/html
$ln -s ~/SOLARIS_ANALYSIS SOLARIS 
```

I want localhost/SOLARIS map to /var/www/html/SOLARIS/WebSimHelper, in the apache config

```sh
$cd /etc/apache2/sit-available
$touch SOLARIS.conf
```

inside SOLARIS.conf

```sh
<VirtualHost *:80>
   ServerAdmin rtang@anl.gov
   DocumentRoot /var/www/html/
   ServerName localhost

   #map localhost/SOLARIS to /var/www/html/SOLARIS/WebSimHelper
   Alias /SOLARIS  /var/www/html/SOLARIS/WebSimHelper

   #set the directory properties
   <Directory /var/www/html/>
      Options Indexes FollowSymLinks 
      AllowOverride None
      Require all granted
      Options +ExecCGI
      AddHandler cgi-script .cgi .py
   </Directory>

   ErrorLog ${APACHE_LOG_DIR}/error.log
   CustomLog ${APACHE_LOG_DIR}/access.log combined

</VirtualHost>
```

then enable the site

```sh
$sudo a2ensite SOLARIS.conf
$sudo systemctl restart apach2.service
```

