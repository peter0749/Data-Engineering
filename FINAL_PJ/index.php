<!DOCTYPE html>
<html>
    <meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />
    <link rel="stylesheet" href="https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/css/bootstrap.min.css">
    <script src="https://ajax.googleapis.com/ajax/libs/jquery/3.3.1/jquery.min.js"></script>
    <script src="https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/js/bootstrap.min.js"></script>
        <body>
            <form action="?" method="post">
                <div class="form-group", align="left">
                <textarea rows="10" cols="80" type="search" id="search" name="search" ><?php if (isset($_POST["search"])) { echo stripslashes($_POST["search"]);  } else { echo "請輸入欲搜尋文章內文"; } ?></textarea>
                    <br/>
                    <label for="number">顯示前幾筆：</label>
                    <input type="number" id="topN" name="topN" min="1" max="1000">
                    <input type="checkbox" id="length_matters" name="length_matters" value="length_matters" checked>比較文章長度
                </div>
                <button type="submit" class="btn btn-default">Go!</button>
            </form>
            <?php
            header("content-type:text/html;charset=utf-8");
            mb_internal_encoding("UTF-8");
            $start_time = microtime(True);
            $search_pattern = " ";
            if (isset($_POST["search"])) {
                $search_pattern = stripslashes($_POST["search"]);
            }
            $normalize_histogram = 0;
            $topN = 1;
            if (isset($_POST["topN"]) && $_POST["topN"]>=0) {
                $topN = $_POST["topN"];
            }
            if (isset($_POST["length_matters"])) {
                $normalize_histogram = ($_POST["length_matters"]==True)?0:1;
            }
            $topN = preg_replace('/[^0-9]/', '', stripslashes($topN));
            if ($topN<1) $topN=1;
            if ($topN>1000) $topN=1000;

            $descriptorspec = array(
                0 => array("pipe", "rb"),  // stdin is a pipe that the child will read from
                1 => array("pipe", "wb"),  // stdout is a pipe that the child will write to
                2 => array("file", "/dev/null", "wb") // stderr is a file to write to
            );
            $env = array(
                'LC_ALL' => 'zh_TW.UTF-8', 
                'LC_CTYPE' => 'zh_TW.UTF-8', 
                'LANG' => 'zh_TW.UTF-8',
                'LANGUAGE' => 'zh_TW.UTF-8'
            );

            $command = "./retrive_topN " . escapeshellarg($topN) . " " . escapeshellarg($normalize_histogram);
            $process = proc_open($command, $descriptorspec, $pipes, NULL, $env);

            if (is_resource($process)) {
                fwrite($pipes[0], $search_pattern);
                fclose($pipes[0]);
                $topN_id_distance = stream_get_contents($pipes[1]);
                fclose($pipes[1]);
                $return_value = proc_close($process);
            }

            if ($return_value==0 || isset($topN_id_distance)) { 
                echo "<div> <table class=\"table table-striped\">";
                echo "<tr> <th> 標題 </th> <th> 差異度 </th> </tr>";
                $rows = explode("\n", $topN_id_distance);
                foreach ( $rows as $line ) {
                    $arr = explode(" ", $line);
                    if (count($arr)!=2) continue;
                    $id = $arr[0];
                    $dist = $arr[1];
                    $command = "./find_news_by_id " . escapeshellarg($id);
                    $process = proc_open($command, $descriptorspec, $pipes, NULL, $env);
                    if (is_resource($process)) {
                        fclose($pipes[0]);
                        $rets = stream_get_contents($pipes[1]);
                        fclose($pipes[1]);
                        $return_val = proc_close($process);
                        $results = explode("\n", $rets);
                    }
                    if (!isset($results) || $return_val!=0) {
                        $results = array(0 => "N/A", 1 => "N/A");
                    }

                    echo "<tr>";
                    echo "<td><a href=" . $results[0] . ">" . $results[1] . "</a></td>";
                    echo "<td>" . $dist . "</td>";
                    echo "</tr>";
                }
                echo "</table></div>";   
            }
            $end_time = microtime(True);
            $elapsed_time = $end_time - $start_time;
            echo "<br/><div align=\"right\">搜尋時間：" . number_format($elapsed_time, 2) . " 秒";
            ?>
        </body>
</html>
