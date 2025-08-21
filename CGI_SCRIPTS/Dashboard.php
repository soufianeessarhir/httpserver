<?php
    session_start();
    
    if (!isset($_SESSION["logined"]))
    {
        header("Location: /CGI_SCRIPTS/Login.php");
        exit;
    }

    if (isset($_POST["logout"]))
    {
        session_unset();
        session_destroy();
        header("Location: /CGI_SCRIPTS/Login.php");
    }

    $theme = isset($_COOKIE['theme']) ? $_COOKIE['theme'] : 'light';

    if (isset($_POST['theme']))
    {
        $newtheme = $_POST['theme'] === 'dark' ? 'dark' : 'light';
        setcookie('theme', $newtheme, time() + (86400 * 30), "/");
        header("Refresh:0");
        exit;
    }

    $username = isset($_SESSION['username']) ? $_SESSION['username'] : 'User';
?>

<!DOCTYPE html>
<html lang="en" data-theme="<?php echo $theme; ?>">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Dashboard - Login Success</title>
    <style>
        :root[data-theme="light"] {
            --bg-color: #f8fafc;
            --card-bg: #ffffff;
            --text-primary: #1e293b;
            --text-secondary: #64748b;
            --success-color: #10b981;
            --border-color: #e2e8f0;
            --shadow: 0 4px 6px -1px rgb(0 0 0 / 0.1);
        }

        :root[data-theme="dark"] {
            --bg-color: #0f172a;
            --card-bg: #1e293b;
            --text-primary: #f8fafc;
            --text-secondary: #cbd5e1;
            --success-color: #10b981;
            --border-color: #475569;
            --shadow: 0 4px 6px -1px rgb(0 0 0 / 0.3);
        }

        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }

        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background-color: var(--bg-color);
            color: var(--text-primary);
            min-height: 100vh;
            display: flex;
            align-items: center;
            justify-content: center;
            padding: 20px;
            transition: all 0.3s ease;
        }

        .success-container {
            background-color: var(--card-bg);
            border: 1px solid var(--border-color);
            border-radius: 16px;
            padding: 40px;
            box-shadow: var(--shadow);
            text-align: center;
            max-width: 500px;
            width: 100%;
            animation: fadeInUp 0.6s ease-out;
        }

        @keyframes fadeInUp {
            from {
                opacity: 0;
                transform: translateY(30px);
            }
            to {
                opacity: 1;
                transform: translateY(0);
            }
        }

        .success-icon {
            width: 80px;
            height: 80px;
            background-color: var(--success-color);
            border-radius: 50%;
            display: flex;
            align-items: center;
            justify-content: center;
            margin: 0 auto 24px;
            animation: scaleIn 0.8s ease-out 0.3s both;
        }

        @keyframes scaleIn {
            from {
                transform: scale(0);
            }
            to {
                transform: scale(1);
            }
        }

        .checkmark {
            color: white;
            font-size: 40px;
            font-weight: bold;
        }

        .success-title {
            font-size: 28px;
            font-weight: 700;
            color: var(--text-primary);
            margin-bottom: 12px;
        }

        .success-message {
            color: var(--text-secondary);
            font-size: 16px;
            margin-bottom: 32px;
            line-height: 1.6;
        }

        .user-greeting {
            background: linear-gradient(135deg, var(--success-color), #059669);
            -webkit-background-clip: text;
            -webkit-text-fill-color: transparent;
            background-clip: text;
            font-size: 20px;
            font-weight: 600;
            margin-bottom: 32px;
        }

        .controls {
            display: flex;
            gap: 16px;
            justify-content: center;
            flex-wrap: wrap;
        }

        .btn {
            padding: 12px 24px;
            border-radius: 8px;
            font-size: 14px;
            font-weight: 600;
            cursor: pointer;
            transition: all 0.3s ease;
            border: none;
            text-decoration: none;
            display: inline-block;
        }

        .btn-theme {
            background-color: var(--card-bg);
            color: var(--text-primary);
            border: 2px solid var(--border-color);
        }

        .btn-theme:hover {
            border-color: var(--success-color);
            color: var(--success-color);
            transform: translateY(-2px);
        }

        .btn-logout {
            background-color: #ef4444;
            color: white;
        }

        .btn-logout:hover {
            background-color: #dc2626;
            transform: translateY(-2px);
        }

        .login-time {
            margin-top: 24px;
            padding-top: 24px;
            border-top: 1px solid var(--border-color);
            color: var(--text-secondary);
            font-size: 14px;
        }

        @media (max-width: 480px) {
            .success-container {
                padding: 32px 24px;
            }
            
            .controls {
                flex-direction: column;
            }
            
            .btn {
                width: 100%;
            }
        }
    </style>
</head>
<body>
    <div class="success-container">
        <div class="success-icon">
            <div class="checkmark">✓</div>
        </div>
        
        <h1 class="success-title">Login Successful!</h1>
        
        <p class="success-message">
            You have been successfully logged into your account.
        </p>
        
        <div class="user-greeting">
            Welcome back, <?php echo htmlspecialchars($username); ?>!
        </div>
        
        <div class="controls">
            <form method="POST" style="display: inline;">
                <button type="submit" name="theme" value="<?php echo $theme === 'light' ? 'dark' : 'light'; ?>" class="btn btn-theme">
                    <?php echo $theme === 'light' ? '🌙 Switch to Dark' : '☀️ Switch to Light'; ?>
                </button>
            </form>
            
            <form method="POST" style="display: inline;">
                <button type="submit" name="logout" class="btn btn-logout">
                    Logout
                </button>
            </form>
        </div>
        
        <div class="login-time">
            Logged in at <?php echo date('F j, Y \a\t g:i A'); ?>
        </div>
    </div>

    <script>
        // Add a subtle animation to the checkmark
        setTimeout(() => {
            const checkmark = document.querySelector('.checkmark');
            checkmark.style.animation = 'pulse 2s infinite';
        }, 1000);
        
        // Add pulse animation
        const style = document.createElement('style');
        style.textContent = `
            @keyframes pulse {
                0%, 100% { transform: scale(1); }
                50% { transform: scale(1.1); }
            }
        `;
        document.head.appendChild(style);
    </script>
</body>
</html>