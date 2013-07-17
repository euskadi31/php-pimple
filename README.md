php-pimple
==========

A small PHP 5.3 dependency injection container (PHP extension)

Creating a container is a matter of instating the ``Pimple`` class:

~~~php
$container = new Pimple();
~~~

As many other dependency injection containers, Pimple is able to manage two
different kind of data: *services* and *parameters*.