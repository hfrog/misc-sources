#
# simple apache runner
#
FROM eboraas/apache-php
RUN apt-get -y update && apt-get -y install curl less net-tools telnet vim wget
COPY index.cgi /var/www/html/
COPY 000-default.conf /etc/apache2/sites-available/
RUN chmod +x /var/www/html/index.cgi \
        && mv /var/www/html/index.html /var/www/html/index.html- \
        && ln -s ../mods-available/cgi.load /etc/apache2/mods-enabled/cgi.load
CMD ["/usr/sbin/apache2ctl", "-D", "FOREGROUND"]
