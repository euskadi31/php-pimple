--TEST--
Debug test
--SKIPIF--
<?php if (!extension_loaded("pimple")) print "skip"; ?>
--FILE--
<?php 

$pimple = new Pimple();

$pimple['time'] = $pimple->share(function() {
    return time();
});

class Service 
{
    protected $hash;

    public function __construct()
    {
        $this->hash = md5(uniqid(''));
    }

    public function getHash()
    {
        return $this->hash;
    }

    public function hello()
    {
        return "Hello";
    }

    public function __toString()
    {
        return "Hello";
    }
}

$pimple = new Pimple(array(
    "foot" => "bar"
));

$pimple['test'] = 123;

$func = function($c) {
    return new Service();
};

$pimple['service'] = function($c) use ($func) {
    static $object;

    if (null == $object) {
        $object = $func($c);
    }

    return $object;
};
/*
$time_start = microtime(true);

for ($i = 0; $i < 100000; $i++) {
    $pimple['service']->getHash();
}
$time_end = microtime(true);

echo $time_end - $time_start . PHP_EOL;
*/
//var_dump($pimple['service']->getHash());
//var_dump($pimple['service']->getHash());

try {
    $pimple['foot'];
    echo "Try OK" . PHP_EOL;
} catch(Exception $e) {
    echo "Try ERROR" . PHP_EOL;
}

try {
    $pimple['test1'];

} catch(Exception $e) {
    echo "Catch OK" . PHP_EOL;
}

assert(isset($pimple['foot']));
assert(!isset($pimple['foot1']));

unset($pimple['foot']);
assert(!isset($pimple['foot']));

echo "Done" . PHP_EOL;

?>
--EXPECT--
Try OK
Catch OK
Done
