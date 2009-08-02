import web
from subprocess import *
urls = ('/upload', 'Upload')

class Upload:
    def GET(self):
        return """<html><head></head><body>
<form method="POST" enctype="multipart/form-data" action="">
<input type="file" name="myfile" />
<br/>
<input type="submit" />
</form>
</body></html>"""

    def POST(self):
        obj = web.input(myfile={})
        filedir = '/Users/david/Devel/arch/avr/code/quickdev16/roms' # change this to the directory you want to store the file in.
        if 'myfile' in obj: 
            web.debug("Upload file %s" % obj['myfile'].filename) 
            filepath = obj.myfile.filename.replace('\\','/')             
            filename = filepath.split('/')[-1] 
            foutname = filedir +'/'+ filename
            web.debug("Write to %s" % foutname) 
            fout = open(foutname,'w') 
            fout.write( obj.myfile.file.read()) 
            fout.close() 
            
            cmd = "ucon64 --port=usb --xsnesram %s " %  foutname
            web.debug("Execute: %s" % cmd) 
            p = Popen(cmd, shell=True, bufsize=128,
                      stdin=PIPE, stdout=PIPE, stderr=PIPE, close_fds=True)            
            stdout,stderr = p.communicate()
            return '''<html><head></head><body>Out: %s <br/>Err: %s</body></html>''' % (
                    stdout.replace("\n","<br/>").replace("\r","<br/>"),
                    stderr.replace("\n","<br/>"))
        raise web.seeother('/upload')

if __name__ == "__main__":
   app = web.application(urls, globals()) 
   app.run()
