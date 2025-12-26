# crawler/renderers.py (create this file if not exists)
from rest_framework.renderers import BaseRenderer

class CSVRenderer(BaseRenderer):
    media_type = 'text/csv'
    format = 'csv'
    charset = 'utf-8'
    render_style = 'text'

    def render(self, data, media_type=None, renderer_context=None):
        if not isinstance(data, list):
            data = []  # Fallback
        csv_content = '\n'.join(data) + '\n'
        return csv_content