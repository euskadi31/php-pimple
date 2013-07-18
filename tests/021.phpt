--TEST--
Test Setting An Invokable Object Should Treat It As Factory
--FILE--
<?php

class Invokable {
    public function __invoke()
    {
        return 'I was invoked';
    }
}

$pimple = new Pimple();
$pimple['invokable'] = new Invokable();

assert('I was invoked' == $pimple['invokable']);

echo "Done" . PHP_EOL;
?>
--EXPECT--
Done