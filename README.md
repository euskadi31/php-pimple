php-pimple [![Build Status](https://travis-ci.org/euskadi31/php-pimple.png)](https://travis-ci.org/euskadi31/php-pimple)
==========

A small PHP 5.3 dependency injection container (PHP extension)


php-pimple is a port of the [Pimple](https://github.com/fabpot/Pimple) library written in PHP.


Creating a container is a matter of instating the ``Pimple`` class:

~~~php
$container = new Pimple();
~~~

As many other dependency injection containers, Pimple is able to manage two
different kind of data: *services* and *parameters*.