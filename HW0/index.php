<!DOCTYPE html>
<html>
    <meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />
    <link rel="stylesheet" href="https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/css/bootstrap.min.css">
    <script src="https://ajax.googleapis.com/ajax/libs/jquery/3.3.1/jquery.min.js"></script>
    <script src="https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/js/bootstrap.min.js"></script>
        <body>
            <form action="?" method="get">
                <div class="form-group", align="left">
                    <label for="search">搜尋：</label>
                    <input type="search" class="form-control" id="search" name="search">
                </div>
                <button type="submit" class="btn btn-default">Go!</button>
            </form>
            <?php
            header("content-type:text/html;charset=utf-8");
            session_start();
            $page_max_row = 50;
            $search_pattern = "一";
            if (isset($_GET["search"])) {
                $search_pattern = stripslashes($_GET["search"]);
            }
            $page = 0;
            if (isset($_GET["page"]) && $_GET["page"]>=0) {
                $page = $_GET["page"];
            }
            $page = preg_replace('/[^0-9]/', '', stripslashes($page));
            $start_row = $page * $page_max_row;
            $end_row = ($page+1) * $page_max_row;
            if (isset($_SESSION['search']) && $_SESSION['search']===$_GET['search'] && isset($_SESSION['tmp_result']) && file_exists($_SESSION['tmp_result'])) {
                $tmp_result = $_SESSION['tmp_result'];
                // echo "Prefetched";
            } else {
                $tmp_result = tempnam("./tmp", "search_sentence_");
                $command = "grep -P \"^" . $search_pattern . "[^\\t]*\" " . "dataset.txt > " . $tmp_result;
                exec($command, $outputs, $return_status);
                $_SESSION['tmp_result'] = $tmp_result;
                $_SESSION['search'] = $search_pattern;
            }

            $handle = fopen($tmp_result, "r");

            echo "<div> <table class=\"table table-striped\">";
            echo "<tr> <th> 來源 </th> <th> 內文 </th> </tr>";
            $cnt = 0;
            while (($line = fgets($handle)) !== false) {
                if ($cnt >= $start_row && $cnt < $end_row) {
                    $arr = explode("\t", $line);
                    echo "<tr>";
                    echo "<td><a href=" . $arr[2] . ">" . $arr[1] . "</a></td>";
                    echo "<td>" . $arr[0] . "</td>";
                    echo "</tr>";
                }
                ++$cnt;
            }
            echo "</table></div>";
            // now $cnt has total line number
            $tot_pages = $cnt / $page_max_row; // floor
            $url = (isset($_SERVER['HTTPS']) && $_SERVER['HTTPS'] === 'on' ? "https" : "http") . "://$_SERVER[HTTP_HOST]" . explode('?',  $_SERVER['REQUEST_URI'], 2 )[0];
            echo "<div align=\"center\">";
            for ($i=0; $i<$tot_pages; ++$i) {
                $newurl = $url . "?search=" . $search_pattern . "&page=" . $i;
                echo "<a href=\"" . $newurl . "\"><button type=\"button\" class=\"btn btn-default\">" . ($i+1) . "</button></a>";
            }
            echo "</div>";
            ?>
        </body>
</html>
