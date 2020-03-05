/**
 * Generated by build-library.php, Please DO NOT modify!
 */

static char* cmpp_library_source_cmpp2 =
    "\n"
    "\n"
    "namespace Swoole\\Coroutine;\n"
    "\n"
    "//短信接口基类\n"
    "abstract class CmppAbstract\n"
    "{\n"
    "\n"
    "    public $errCode = 0;\n"
    "    public $errMsg = \"\";\n"
    "    public $ext = NULL;\n"
    "    public $setting = [];\n"
    "    public $sendChannel = NULL;\n"
    "\n"
    "    abstract function login($ip, $port, $uname, $pwd, float $timeout = -1);\n"
    "\n"
    "    abstract function recv();\n"
    "\n"
    "    abstract function logout();\n"
    "\n"
    "    abstract function realSubmit($mobile, $unicode_text, $ext, float $timeout = -1, int $udhi = -1, int $smsTotalNumber = 1, int $i = 1);\n"
    "\n"
    "    public function submit($mobile, $text, $ext, float $timeout = -1, int $udhi = -1)\n"
    "    {\n"
    "        $smsContentLength = mb_strlen($text, 'UTF-8');\n"
    "        if ($smsContentLength <= 70) { // 短短信\n"
    "            $unicode_text = mb_convert_encoding($text, \"UCS-2BE\", \"UTF-8\");\n"
    "            $ret = $this->realSubmit($mobile, $unicode_text, $ext, $timeout);\n"
    "            if ($ret === FALSE) {\n"
    "                return FALSE;\n"
    "            }\n"
    "            $seqArr[] = $ret;\n"
    "        } else { //长短信时\n"
    "            if ($udhi == -1 || $udhi >= 255) {\n"
    "                $this->errMsg = \"udhi param error ($udhi)\";\n"
    "                return FALSE;\n"
    "            }\n"
    "            $seqArr = [];\n"
    "            $smsTotalNumber = ceil($smsContentLength / 67);\n"
    "            for ($i = 0; $i < $smsTotalNumber; $i++) {\n"
    "                $content = mb_substr($text, $i * 67, 67, 'UTF-8');\n"
    "                $unicode_text = mb_convert_encoding($content, \"UCS-2BE\", 'UTF-8');\n"
    "                $ret = $this->realSubmit($mobile, $unicode_text, $ext, $timeout, $udhi, $smsTotalNumber, $i + 1);\n"
    "                if ($ret === FALSE) {\n"
    "                    return FALSE;\n"
    "                }\n"
    "                $seqArr[] = $ret;\n"
    "            }\n"
    "        }\n"
    "        return $seqArr;\n"
    "    }\n"
    "\n"
    "    public function syncErr()\n"
    "    {\n"
    "        $this->errCode = $this->ext->errCode;\n"
    "        $this->errMsg = $this->ext->errMsg;\n"
    "    }\n"
    "\n"
    "}\n"
    "\n"
    "class Cmpp2 extends CmppAbstract\n"
    "{//cmpp容器类\n"
    "//    private $recvCid = -1;\n"
    "//    private $submitting = 0;\n"
    "//    private $recvWating = 0;\n"
    "//    private $inbatch = 0;\n"
    "\n"
    "    public function __construct($set)\n"
    "    {\n"
    "        $this->setting = $set;\n"
    "    }\n"
    "\n"
    "    private function connBroken()\n"
    "    {//连接断开 销毁实例 结束协程\n"
    "        $this->ext->close();\n"
    "        $this->ext = NULL;\n"
    "    }\n"
    "\n"
    "    public function login($ip, $port, $uname, $pwd, float $timeout = -1)\n"
    "    {\n"
    "        if (is_null($this->ext)) {\n"
    "            $this->ext = new \\Swoole\\Coroutine\\Cmpp($this->setting);\n"
    "            $ret = $this->ext->dologin($ip, $port, $uname, $pwd, $timeout);\n"
    "            if (is_array($ret) && $ret['Status'] == 0) {\n"
    "                $this->sendChannel = new \\Swoole\\Coroutine\\Channel(3);\n"
    "                //心跳协程\n"
    "                \\Swoole\\Coroutine::create(function () {\n"
    "                    while (1) {\n"
    "                        \\Swoole\\Coroutine::sleep($this->setting['active_test_interval']);\n"
    "                        if (is_null($this->ext)) {\n"
    "                            break; //结束协程\n"
    "                        }\n"
    "                        $pingData = $this->ext->activeTest();\n"
    "                        if ($pingData === FALSE) {\n"
    "                            return $this->ext->close();\n"
    "                        }\n"
    "                        if ($pingData) {\n"
    "                            $this->sendChannel->push($pingData);\n"
    "                        }\n"
    "                    }\n"
    "                });\n"
    "                //专门的发送协程\n"
    "                \\Swoole\\Coroutine::create(function () {\n"
    "                    while (1) {\n"
    "                        $pack = $this->sendChannel->pop();\n"
    "                        if (is_null($this->ext)) {\n"
    "                            break; //结束协程\n"
    "                        }\n"
    "                        $this->ext->sendOnePack($pack);\n"
    "                    }\n"
    "                });\n"
    "                return $ret;\n"
    "            } else {\n"
    "                $this->errMsg = $this->ext->errMsg;\n"
    "                $this->errCode = $this->ext->errCode;\n"
    "                $this->ext = null;\n"
    "                return $ret;\n"
    "            }\n"
    "        } else {\n"
    "            $this->errMsg = \"the connection is connected\";\n"
    "            $this->errCode = CMPP_CONN_CONNECTED;\n"
    "            return FALSE;\n"
    "        }\n"
    "    }\n"
    "\n"
    "    public function realSubmit($mobile, $unicode_text, $ext, float $timeout = -1, int $udhi = -1, int $smsTotalNumber = 1, int $i = 1)\n"
    "    {\n"
    "        again:\n"
    "        if (is_null($this->ext)) {\n"
    "            return FALSE;\n"
    "        }\n"
    "        $ret = $this->ext->submit($mobile, $unicode_text, $ext, $udhi, $smsTotalNumber, $i);\n"
    "        if ($ret === FALSE) {\n"
    "            $this->syncErr();\n"
    "            $this->connBroken();\n"
    "            return FALSE;\n"
    "        }\n"
    "        if (is_double($ret)) {\n"
    "            \\Swoole\\Coroutine::sleep($ret);\n"
    "            goto again;\n"
    "        }\n"
    "\n"
    "        $cRet = $this->sendChannel->push($ret['packdata'], $timeout);\n"
    "        if ($cRet === FALSE) {\n"
    "            $this->errCode = $this->sendChannel->errCode;\n"
    "            return FALSE;\n"
    "        }\n"
    "        return $ret['sequence_id'];\n"
    "    }\n"
    "\n"
    "    public function recv(float $timeout = -1)\n"
    "    {\n"
    "        again:\n"
    "        $ret = $this->ext->recvOnePack($timeout);\n"
    "        if ($ret === false) {\n"
    "            $this->syncErr();\n"
    "            if ($this->errCode === CMPP_CONN_BROKEN) {\n"
    "                $this->connBroken();\n"
    "            }\n"
    "            return FALSE;\n"
    "        }\n"
    "        switch ($ret['Command']) {\n"
    "            case CMPP2_ACTIVE_TEST_RESP:\n"
    "            case CMPP2_TERMINATE_RESP://对端主动断开给回复的包\n"
    "                $this->sendChannel->push($ret[\"packdata\"]);\n"
    "                goto again;\n"
    "            case CMPP2_DELIVER:\n"
    "                $this->sendChannel->push($ret[\"packdata\"]);\n"
    "                unset($ret[\"packdata\"]);\n"
    "                return $ret;\n"
    "            case CMPP2_SUBMIT_RESP:\n"
    "                return $ret;\n"
    "            default :\n"
    "                return false;\n"
    "//                throw new \\Exception(\"error command \" . $ret['Command']);\n"
    "        }\n"
    "    }\n"
    "\n"
    "    /*\n"
    "     * 发送term包 收到回复的term resp后 recv返回false 断开连接\n"
    "     */\n"
    "\n"
    "    public function logout()\n"
    "    {\n"
    "        $packdata = $this->ext->logout();\n"
    "        $this->sendChannel->push($packdata);\n"
    "    }\n"
    "\n"
    "}\n"
    "\n"
    "class Cmpp3 extends Cmpp2\n"
    "{\n"
    "\n"
    "    public function __construct($set)\n"
    "    {\n"
    "        $set['protocal'] = 'cmpp3';\n"
    "        parent::__construct($set);\n"
    "    }\n"
    "\n"
    "}\n"
    "\n"
    "class SgipClient extends CmppAbstract\n"
    "{//sgip容器类\n"
    "\n"
    "    public function __construct($set)\n"
    "    {\n"
    "        $this->setting = $set;\n"
    "    }\n"
    "\n"
    "    public function login($ip, $port, $uname, $pwd, float $timeout = -1)\n"
    "    {\n"
    "        if (is_null($this->ext)) {\n"
    "            $this->ext = new \\Swoole\\Coroutine\\Sgip($this->setting);\n"
    "            $ret = $this->ext->bind($ip, $port, $uname, $pwd, $timeout);\n"
    "            if (is_array($ret) && $ret['Result'] == 0) {\n"
    "                $this->sendChannel = new \\Swoole\\Coroutine\\Channel(3);\n"
    "                //专门的发送协程\n"
    "                \\Swoole\\Coroutine::create(function () {\n"
    "                    while (1) {\n"
    "                        $pack = $this->sendChannel->pop();\n"
    "                        if (is_null($this->ext)) {\n"
    "                            break; //结束协程\n"
    "                        }\n"
    "                        $this->ext->sendOnePack($pack);\n"
    "                    }\n"
    "                });\n"
    "                return $ret;\n"
    "            } else {\n"
    "                $this->syncErr();\n"
    "                $this->ext = null;\n"
    "                return $ret;\n"
    "            }\n"
    "        } else {\n"
    "            $this->errMsg = \"the connection is connected\";\n"
    "            $this->errCode = CMPP_CONN_CONNECTED;\n"
    "            return FALSE;\n"
    "        }\n"
    "    }\n"
    "\n"
    "    public function realSubmit($mobile, $unicode_text, $ext, float $timeout = -1, int $udhi = -1, int $smsTotalNumber = 1, int $i = 1)\n"
    "    {\n"
    "        if (substr($mobile, 0, 2) !== \"86\") {\n"
    "            $mobile = \"86\" . $mobile;\n"
    "        }\n"
    "        $ret = $this->ext->submit($mobile, $unicode_text, $ext, $udhi, $smsTotalNumber, $i);\n"
    "        if ($ret === FALSE) {\n"
    "            $this->errCode = CMPP_CONN_BROKEN;\n"
    "            return FALSE;\n"
    "        }\n"
    "\n"
    "        $cRet = $this->sendChannel->push($ret['packdata'], $timeout);\n"
    "        if ($cRet === FALSE) {\n"
    "            $this->errCode = $this->sendChannel->errCode;\n"
    "            return FALSE;\n"
    "        }\n"
    "        return $ret['sequence_id'];\n"
    "    }\n"
    "\n"
    "    public function recv(float $timeout = -1)\n"
    "    {\n"
    "        again:\n"
    "        $ret = $this->ext->recvOnePack($timeout);\n"
    "        if ($ret === false) {\n"
    "            $this->syncErr();\n"
    "            if ($this->ext->errCode != SOCKET_ETIMEDOUT) {\n"
    "                $this->errCode = CMPP_CONN_BROKEN;\n"
    "            }\n"
    "            return FALSE;\n"
    "        }\n"
    "        switch ($ret['Command']) {\n"
    "            case SGIP_UNBIND_RESP://对端主动断开给回复的包\n"
    "                $this->sendChannel->push($ret[\"packdata\"]);\n"
    "                goto again;\n"
    "            case SGIP_SUBMIT_RESP:\n"
    "                return $ret;\n"
    "            default :\n"
    "                $this->errCode = CMPP_CONN_BROKEN;\n"
    "                return FALSE;\n"
    "//                throw new \\Exception(\"error command \" . $ret['Command']);\n"
    "        }\n"
    "    }\n"
    "\n"
    "    /*\n"
    "     * 发送term包 收到回复的term resp后 recv返回false 断开连接\n"
    "     */\n"
    "\n"
    "    public function logout()\n"
    "    {\n"
    "        $this->sendChannel->push(\\Swoole\\Coroutine\\Sgip::unbindPack());\n"
    "    }\n"
    "\n"
    "}\n"
    "\n"
    "class SgipServer extends Server\n"
    "{\n"
    "    /*\n"
    "     * array(\n"
    "     *    client_ips = ['a','b','c'],\n"
    "     * *    client_ips = '0.0.0.0'代表不限制\n"
    "     *    max_connection = 10,//每个ip最多多少个连接\n"
    "      'bind_timeout'=>5,//tcp连上后 多久不发bind包就close连接。\n"
    "     * )\n"
    "     */\n"
    "\n"
    "    public static $sgipSet = [];\n"
    "    public static $connLimit = [];\n"
    "\n"
    "    public function sgipSet($arr)\n"
    "    {\n"
    "        self::$sgipSet = $arr;\n"
    "    }\n"
    "\n"
    "    public function start(): bool\n"
    "    {\n"
    "        if (!isset(self::$sgipSet['client_ips']) || !isset(self::$sgipSet['max_connection']) || !isset(self::$sgipSet['bind_timeout'])) {\n"
    "            throw new \\Exception(\"sgip set error\");\n"
    "        }\n"
    "        $this->running = true;\n"
    "        if ($this->fn == null) {\n"
    "            $this->errCode = SOCKET_EINVAL;\n"
    "            return false;\n"
    "        }\n"
    "        $socket = $this->socket;\n"
    "        $this->setting = array_merge($this->setting, [\n"
    "            'open_length_check' => 1,\n"
    "            'package_length_type' => 'N',\n"
    "            'package_length_offset' => 0, //第N个字节是包长度的值\n"
    "            'package_body_offset' => 0, //第几个字节开始计算长度\n"
    "            'package_max_length' => 2000000, //协议最大长度\n"
    "            'open_tcp_nodelay' => true //TCP底层合并传输,true表示关闭\n"
    "        ]);\n"
    "        if (!$socket->setProtocol($this->setting)) {\n"
    "            $this->errCode = SOCKET_EINVAL;\n"
    "            return false;\n"
    "        }\n"
    "\n"
    "        while ($this->running) {\n"
    "            /** @var $conn Socket */\n"
    "            $conn = $socket->accept();\n"
    "\n"
    "            //IP限制\n"
    "            $peer = $conn->getpeername();\n"
    "            $client_ips = self::$sgipSet['client_ips'];\n"
    "            if ($client_ips != \"0.0.0.0\") {\n"
    "                if (!in_array($peer['address'], self::$sgipSet['client_ips'])) {\n"
    "                    $conn->close();\n"
    "                    continue;\n"
    "                }\n"
    "            }\n"
    "\n"
    "            //连接数限制\n"
    "            @self::$connLimit[$peer['address']] ++;\n"
    "            if (self::$connLimit[$peer['address']] > self::$sgipSet['max_connection']) {\n"
    "                $conn->close();\n"
    "                self::$connLimit[$peer['address']] --;\n"
    "                continue;\n"
    "            }\n"
    "\n"
    "            if ($conn) {\n"
    "                //accept会产生新的socket，此处设置粘包选项\n"
    "                if (!$conn->setProtocol($this->setting)) {\n"
    "                    $this->errCode = SOCKET_EINVAL;\n"
    "                    return false;\n"
    "                }\n"
    "                if (\\Swoole\\Coroutine::create($this->fn, new SGIPConnection($conn)) < 0) {\n"
    "                    goto _wait;\n"
    "                }\n"
    "            } else {\n"
    "                if ($socket->errCode == SOCKET_EMFILE or $socket->errCode == SOCKET_ENFILE) {\n"
    "                    _wait:\n"
    "                    \\Swoole\\Coroutine::sleep(1);\n"
    "                    continue;\n"
    "                } elseif ($socket->errCode == SOCKET_ETIMEDOUT) {\n"
    "                    continue;\n"
    "                } elseif ($socket->errCode == SOCKET_ECANCELED) {\n"
    "                    break;\n"
    "                } else {\n"
    "                    trigger_error(\"accept failed, Error: {$socket->errMsg}[{$socket->errCode}]\", E_USER_WARNING);\n"
    "                    break;\n"
    "                }\n"
    "            }\n"
    "        }\n"
    "\n"
    "        return true;\n"
    "    }\n"
    "\n"
    "    public function OnConnect(callable $fn)\n"
    "    {\n"
    "        $this->fn = $fn;\n"
    "    }\n"
    "\n"
    "}\n"
    "\n"
    "class SgipConnection extends Server\\Connection\n"
    "{\n"
    "\n"
    "    public $sendChannel = null;\n"
    "    private $status = \"START\";\n"
    "    public $peerName = \"\";\n"
    "\n"
    "    public function recv(float $timeout = 0)\n"
    "    {\n"
    "        if (is_null($this->sendChannel)) {\n"
    "            $this->sendChannel = new \\Swoole\\Coroutine\\Channel(3);\n"
    "            //专门的发送协程\n"
    "            \\Swoole\\Coroutine::create(function () {\n"
    "                while (1) {\n"
    "                    $pack = $this->sendChannel->pop();\n"
    "                    $ret = $this->socket->sendAll($pack, -1);\n"
    "                    if ($ret === false) {//链接出错\n"
    "                        return NULL;\n"
    "                    }\n"
    "                }\n"
    "            });\n"
    "        }\n"
    "        if ($this->status == \"START\") {\n"
    "            //bind_timeout\n"
    "            go(function() {\n"
    "                \\Swoole\\Coroutine::sleep(\\Co\\SgipServer::$sgipSet['bind_timeout']);\n"
    "                if ($this->status == \"START\") {\n"
    "                    $this->socket->close();\n"
    "                }\n"
    "            });\n"
    "        }\n"
    "        again:\n"
    "        $raw = $this->socket->recvPacket($timeout);\n"
    "        if (empty($raw)) {\n"
    "            $peer = $this->socket->getpeername();\n"
    "            \\Co\\SgipServer::$connLimit[$peer['address']] --;\n"
    "            $this->socket->close();\n"
    "            return $raw;\n"
    "        }\n"
    "        $arr = \\Swoole\\Coroutine\\Sgip::parseServerRecv($raw);\n"
    "        switch ($arr['Command']) {\n"
    "            case SGIP_CONNECT:\n"
    "                $this->status = \"BIND\";\n"
    "                $this->sendChannel->push($arr['packdata']); //回复resp\n"
    "                goto again;\n"
    "            case SGIP_UNBIND://继续接收，直到收到tcp的fin\n"
    "                $this->status = \"UNBIND\";\n"
    "                $this->sendChannel->push($arr['packdata']); //回复resp\n"
    "                goto again;\n"
    "            case SGIP_UNBIND_RESP:\n"
    "                $this->status = \"UNBIND\";\n"
    "                $this->socket->close();\n"
    "                goto again;\n"
    "            case SGIP_REPORT:\n"
    "                $this->sendChannel->push($arr['packdata']); //回复resp\n"
    "                unset($arr['packdata']);\n"
    "                return $arr;\n"
    "            case SGIP_DELIVER:\n"
    "                $this->sendChannel->push($arr['packdata']); //回复resp\n"
    "                unset($arr['packdata']);\n"
    "                return $arr;\n"
    "\n"
    "            default:\n"
    "                return NULL;\n"
    "//                throw new \\Exception(\"error command \" . $arr['Command']);\n"
    "        }\n"
    "    }\n"
    "\n"
    "    public function logout()\n"
    "    {\n"
    "        $this->sendChannel->push(\\Swoole\\Coroutine\\Sgip::unbindPack());\n"
    "    }\n"
    "\n"
    "}\n"
    "\n"
    "class_alias(\"Swoole\\\\Coroutine\\\\SgipClient\", \"Co\\\\SgipClient\", true);\n"
    "class_alias(\"Swoole\\\\Coroutine\\\\SgipServer\", \"Co\\\\SgipServer\", true);\n"
    "class_alias(\"Swoole\\\\Coroutine\\\\SgipConnection\", \"Co\\\\SgipConnection\", true);\n"
    "class_alias(\"Swoole\\\\Coroutine\\\\Cmpp2\", \"Co\\\\Cmpp2\", true);\n"
    "class_alias(\"Swoole\\\\Coroutine\\\\Cmpp3\", \"Co\\\\Cmpp3\", true);\n";

void php_cmpp_load_library();

