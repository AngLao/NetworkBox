#ifndef _RESPONSE_HTML_
#define _RESPONSE_HTML_

static const char root_html[] = "<html>\
                    <head>\
                        <style>\
                            body {\
                                font-family: Arial, sans-serif;\
                                background-color: #f4f4f4;\
                                text-align: center;\
                                margin: 50px;\
                            }\
                            h1 {\
                                color: #333;\
                            }\
                            p {\
                                color: #666;\
                            }\
                            form {\
                                margin-top: 20px;\
                                display: inline-block;\
                                text-align: left;\
                            }\
                            label {\
                                display: block;\
                                margin-bottom: 5px;\
                            }\
                            input {\
                                width: 200px;\
                                padding: 5px;\
                                margin-bottom: 10px;\
                            }\
                            input[type=submit] {\
                                background-color: #4CAF50;\
                                color: white;\
                                cursor: pointer;\
                            }\
                        </style>\
                        <script>\
                            document.addEventListener('DOMContentLoaded', function () {\
                                var form = document.getElementById('login-form');\
                                form.addEventListener('submit', function (event) {\
                                    event.preventDefault();\
                                    var username = document.getElementById('username').value;\
                                    var password = document.getElementById('password').value;\
                                    var xhr = new XMLHttpRequest();\
                                    xhr.open('POST', '/login', true);\
                                    var data = 'username=' + encodeURIComponent(username) + '&password=' + encodeURIComponent(password);\
                                    xhr.send(data);\
                                });\
                            });\
                        </script>\
                    </head>\
                    <body>\
                        <h1>Hello Customer !</h1>\
                        <p>Current IP Address: %s</p>\
                        <p>Current IP port: %s</p>\
                        <form id='login-form'>\
                            <label for='username'>Username:</label>\
                            <input type='text' id='username' name='username'><br>\
                            <label for='password'>Password:</label>\
                            <input type='password' id='password' name='password'><br>\
                            <input type='submit' value='Submit'>\
                        </form>\
                    </body>\
                    </html>";

#endif
