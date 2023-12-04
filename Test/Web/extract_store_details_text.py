import urllib.request
from html.parser import HTMLParser

class DescriptionParser(HTMLParser):
    def __init__(self):
        super().__init__()
        self.recording = 0  # Use a counter to track depth of nested <li> tags
        self.data = []

    def handle_starttag(self, tag, attrs):
        if tag == 'li':
            for name, value in attrs:
                if name == 'class' and 'description' in value:
                    self.recording += 1  # Start recording when entering <li class="description">
            if self.recording > 0:
                self.recording += 1  # Increment counter for nested <li>

    def handle_endtag(self, tag):
        if tag == 'li' and self.recording > 0:
            self.recording -= 1  # Decrement counter when exiting <li>

    def handle_data(self, data):
        if self.recording > 1:  # Record data only if inside the correct <li> block
            self.data.append(data)

def extract_description_from_url(url):
    parser = DescriptionParser()
    req = urllib.request.Request(url, headers={'User-Agent': 'Mozilla/5.0'})
    with urllib.request.urlopen(req) as response:
        html = response.read().decode('utf-8')
        parser.feed(html)
    return ''.join(parser.data).strip()

url = "https://www.daz3d.com/amelia-9-hd"  # Example DAZ store URL
description_text = extract_description_from_url(url)

print(description_text)
