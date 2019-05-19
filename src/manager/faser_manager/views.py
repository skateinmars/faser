from faser_manager import app
from faser_manager.auth import auth

@app.route('/')
@auth.login_required
def index():
    return ''
